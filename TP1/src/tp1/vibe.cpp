
#include "tp1/common.hpp"

// local implementation for ViBe segmentation algorithm
struct ViBe_impl : ViBe {
    ViBe_impl(size_t N, size_t R, size_t nMin, size_t nSigma);
    virtual void initialize(const cv::Mat& oInitFrame);
    virtual void apply(const cv::Mat& oCurrFrame, cv::Mat& oOutputMask);
    const size_t m_N; //< internal ViBe parameter; number of samples to keep in each pixel model
    const size_t m_R; //< internal ViBe parameter; maximum color distance allowed between RGB samples for matching
    const size_t m_nMin; //< internal ViBe parameter; required number of matches for background classification
    const size_t m_nSigma; //< internal ViBe parameter; model update rate

    // @@@@ ADD ALL REQUIRED DATA MEMBERS FOR BACKGROUND MODEL HERE
    std::vector<std::vector<cv::Vec3b>> background;

};

std::shared_ptr<ViBe> ViBe::createInstance(size_t N, size_t R, size_t nMin, size_t nSigma) {
    return std::shared_ptr<ViBe>(new ViBe_impl(N,R,nMin,nSigma));
}

ViBe_impl::ViBe_impl(size_t N, size_t R, size_t nMin, size_t nSigma) :
    m_N(N),
    m_R(R),
    m_nMin(nMin),
    m_nSigma(nSigma) {}


void ViBe_impl::initialize(const cv::Mat& oInitFrame) {
    CV_Assert(!oInitFrame.empty() && oInitFrame.isContinuous() && oInitFrame.type()==CV_8UC3);

    // ... @@@@@ TODO

    // hint: we work with RGB images, so the type of one pixel is a "cv::Vec3b"! (i.e. three uint8_t's are stored per pixel)


    for(int i = 1; i < oInitFrame.rows-1; i++)
    {
        for(int j = 1; j < oInitFrame.cols-1; ++j)
        {
            std::vector<cv::Vec3b> tmp;
            cv::Vec3b values[] = {oInitFrame.at<cv::Vec3b>(i-1,j-1), oInitFrame.at<cv::Vec3b>(i-1,j), oInitFrame.at<cv::Vec3b>(i-1,j+1), oInitFrame.at<cv::Vec3b>(i,j-1),
                                  oInitFrame.at<cv::Vec3b>(i,j+1), oInitFrame.at<cv::Vec3b>(i+1,j-1), oInitFrame.at<cv::Vec3b>(i+1,j), oInitFrame.at<cv::Vec3b>(i+1,j+1)};
            int ran;
            for(int k = 0; k < 20; ++k)
            {
                ran = rand() % 8;
                tmp.push_back(values[ran]);
            }
            background.push_back(tmp);
        }
    }
}
bool isInSphere(const cv::Vec3b& pix, const cv::Vec3b& samples, int seuil){
    return (sqrt(pow(pix.val[0]-samples.val[0],2) + pow(pix.val[1]-samples.val[1],2) + pow(pix.val[2]-samples.val[2],2)) <= seuil);
}

void ViBe_impl::apply(const cv::Mat& oCurrFrame, cv::Mat& oOutputMask) {
    CV_Assert(!oCurrFrame.empty() && oCurrFrame.isContinuous() && oCurrFrame.type()==CV_8UC3);
    oOutputMask.create(oCurrFrame.size(),CV_8UC1); // output is binary, but always stored in a byte (so output values are either '0' or '255')

    // ... @@@@@ TODO
    int coo = 0;
    for(int i = 1; i < oCurrFrame.rows-1; i++)
    {
        for(int j = 1; j < oCurrFrame.cols-1; ++j)
        {
            int nbOk = 0, counter = 0;
            coo = (i-1)*(oCurrFrame.cols-2)+j-1;
            while (nbOk < 2 && counter < 20){
                if(isInSphere(background.at(coo).at(counter++), oCurrFrame.at<cv::Vec3b>(i,j), 20)){
                    nbOk++;
                }
            }
            if(nbOk == 2)
            {
                oOutputMask.at<uchar>(i,j) = 0;
                int ran = rand() % 16;
                cv::Vec3b newValue = oCurrFrame.at<cv::Vec3b>(i,j);
                if(!ran)
                {
                    background.at(coo).at(rand() % 20) = newValue;
                }

                if(i != 1 && i != oCurrFrame.rows -2 && j != 1 && j != oCurrFrame.cols -2)
                {
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i-2)*(oCurrFrame.cols-2)+j-2).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i-2)*(oCurrFrame.cols-2)+j-1).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i-2)*(oCurrFrame.cols-2)+j).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i-1)*(oCurrFrame.cols-2)+j-2).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i-1)*(oCurrFrame.cols-2)+j).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i)*(oCurrFrame.cols-2)+j-2).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i)*(oCurrFrame.cols-2)+j-1).at(rand()%20) = newValue;
                    ran = rand() % 16;
                    if(!ran)
                        background.at((i)*(oCurrFrame.cols-2)+j).at(rand()%20) = newValue;
                }


            } else {
                oOutputMask.at<uchar>(i,j) = 255;
            }
        }
    }

    // hint: we work with RGB images, so the type of one pixel is a "cv::Vec3b"! (i.e. three uint8_t's are stored per pixel)

}
