#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <set>


#define NB_PERSONS 40
#define NB_IMG_PER_PERSON 9
#define DATASET NB_IMG_PER_PERSON * NB_PERSONS
#define IMG_W 92
#define IMG_H 112
#define K 300
#define DISTANCE_THRESHOLD 10000
#define DISTANCE 1 //0 = Mahalanobis, 1 = Euclidian

void fillMat(const std::vector<double>& a, const std::vector<double>& b, cv::Mat& mat){
    for(int i = 0; i < a.size(); ++i)
    {
        mat.at<double>(0,i) = a.at(i);
        mat.at<double>(1,i) = b.at(i);
    }
}

double dist(const std::vector<double>& currImgWi, const std::vector<double>& datasetImgWi){
    if(DISTANCE == 0)
    {
        cv::Mat mat(currImgWi.size(), 2, CV_64F);
        fillMat(currImgWi, datasetImgWi, mat);
        cv::Mat matT;
        cv::transpose(mat, matT);
        cv::Mat cov = mat * matT;
        std::cout << cov<< std::endl;

        cv::Mat res;
        cv::invert(cov, res, cv::DECOMP_LU);
        return cv::Mahalanobis(mat.col(0), mat.col(1), res);
    }
    else
    {
        double res = 0;
        for(int i = 0; i < currImgWi.size(); ++i)
            res += pow(datasetImgWi.at(i) - currImgWi.at(i),2);
        return res;
    }
}

int main()
{
    cv::Mat avgImg;
    avgImg.create(IMG_H, IMG_W, CV_64F);
    std::map<int, std::string> mapData;
    std::map<int, cv::Mat> faces;
    for(int i = 1; i <= NB_PERSONS; ++i)
    {
        for(int j = 1; j <= NB_IMG_PER_PERSON; ++j)
        {
            std::stringstream ss;
            std::string path = "/home/pierre/Dev/INF6803/TP3/DATA/training/";
            ss << path << "s" << i << "_" << j << ".pgm";
            cv::Mat img = cv::imread(ss.str(), cv::IMREAD_GRAYSCALE);
            int index = (i-1) * NB_IMG_PER_PERSON + j - 1;
            mapData.insert(std::pair<int,std::string>(index, ss.str()));
            faces.insert(std::pair<int, cv::Mat>(index, img));
            cv::accumulate(img, avgImg);
        }
    }
    avgImg /= DATASET;
    avgImg.convertTo(avgImg, CV_8UC1); //à voir si grave

    cv::Mat matrix(IMG_H*IMG_W, DATASET, CV_64F);
    for(int i = 0; i < faces.size(); ++i)
    {
        cv::subtract(faces.at(i), avgImg, faces.at(i));
        faces.at(i) = faces.at(i).reshape(0,IMG_H*IMG_W);
        faces.at(i).col(0).copyTo(matrix.col(i));

    }

    //transpose matrix
    cv::Mat matrixT;
    cv::transpose(matrix, matrixT);

    //compute covariance matrix
    //matrixT * matrix instead of matrix * matrixT : to limit the dimensionnality
    cv::Mat cov, mu;
    cov = matrixT * matrix;
//    cv::calcCovarMatrix(matrixT, cov, mu, CV_COVAR_NORMAL | CV_COVAR_COLS);

    //compute eigenvalues and eigenvectors
    cv::Mat eVa, eVe;
    cv::eigen(cov, eVa, eVe);



    //real Eigenvector of matrix * matrixT
    cv::Mat realEve(IMG_H*IMG_W, K, CV_64F);
    for(int i = 0; i < K; ++i){
        realEve.col(i) = matrix * eVe.col(i);
        cv::normalize(realEve.col(i),realEve.col(i),1);
    }

    //debug eigenvectors
    /**
    double summ = 0;
    for(int i = 0; i < IMG_H*IMG_W; ++i)
    {
        summ += pow(realEve.at<double>(0,i),2);
        std::cout << realEve.at<double>(0,i) << " ";
    }
    std::cout << std::endl << sqrt(summ) << std::endl;
    return 0;
    **/

    //compute wi for all t1raining set images
    std::vector<std::vector<double>> dataset_wi;
    for(int i = 0; i < faces.size(); ++i)
    {
        std::vector<double> tmp;
        for(int j = 0; j < K; ++j)
        {
            cv::Mat ui = realEve.col(j);
            cv::transpose(ui,ui);
            ui.convertTo(ui, CV_64F);
            faces.at(i).convertTo(faces.at(i), CV_64F);
            cv::Mat m =  ui*faces.at(i);
            tmp.push_back(m.at<double>(0,0));
        }
        dataset_wi.push_back(tmp);
    }

    for(int i = 0; i < dataset_wi.size(); ++i)
    {
        for(int j = 0; j < dataset_wi.at(i).size(); ++j)
        {
            std::cout << dataset_wi.at(i).at(j) << " ";
        }
        std::cout << std::endl;
    }

    //load new image
    cv::Mat image = cv::imread( "/home/pierre/Dev/INF6803/TP3/DATA/test/s26_10.pgm", cv::IMREAD_GRAYSCALE);
    cv::subtract(image, avgImg, image);
    image = image.reshape(0,IMG_H*IMG_W);
    image.convertTo(image, CV_64F);


    //compute omega (and phi hat even if no needed)
    cv::Mat phi_hat(IMG_H*IMG_W, 1, CV_64F);
    std::vector<double> omega;
    for(int i = 0; i < K; ++i)
    {
        cv::Mat tmp = realEve.col(i);
        cv::Mat tmpT, wi;
        tmp.convertTo(tmp, CV_64F);
        cv::transpose(tmp,tmpT);
        wi = tmpT * image;
        omega.push_back(wi.at<double>(0,0));
        cv::Mat res_ite;
        double wi_fact = wi.at<double>(0,0);
        res_ite = wi_fact * tmp;
        cv::accumulate(res_ite, phi_hat);
    }

    for(int i = 0;  i < omega.size(); ++i)
        std::cout << omega.at(i) << " ";
    std::cout << std::endl;
    //check if the face is known
    double best_dist = 1000000000, best_idx = -1;
    for(int i = 0; i < dataset_wi.size(); ++i)
    {
        double res = dist(omega, dataset_wi.at(i));
        std::cout << mapData.at(i) << " = " << res << std::endl;
        if(res < best_dist){
            best_dist = res;
            best_idx = i;
        }
    }

    std::cout << "(" << best_dist << ", " << best_idx << ")" << " => " << mapData.at(best_idx) << std::endl;

//    imshow("OL", avgImg);
    cv::waitKey(0);
    cv::namedWindow("OL");
    return 0;
}
