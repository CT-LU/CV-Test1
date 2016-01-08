#include "CubeGenerator.hpp"

using namespace cv;
using namespace std;

CubeGenerator::CubeGenerator(){
}

void CubeGenerator::generateCube(const Rect &hand_roi, const Mat &depth_frame, int cube[6]){
    //Create Cube
    //Get Histogram in Depth Image
    Mat depth_hist, mask;
    //int channels = depth_frame.channels();
    int histSize = 256;
    float range[] = {0,256};
    const float* histRange = range;
    //Create Mask
    mask = Mat(depth_frame.size(), CV_8UC1, Scalar::all(0));
    mask(hand_roi).setTo(Scalar::all(255));

    //calculate histogram of depth image
    calcHist(&depth_frame, 1, 0, mask, depth_hist, 1, &histSize, &histRange);
    
    //show Hist
    //cout << "Depth Hist -> rows: " << depth_hist.rows << "  cols: " << depth_hist.cols << "  channels: " << depth_hist.channels() << endl;
    
    //Get first and second set in Hist
    int first_idx = 0;
    int first_value = 0;
    int second_idx = 0;
    int second_value = 0;
    int depth_idx = -1;
    for(int i = 15; i < 201; i++){
        int hist_value_ori = saturate_cast<int>(depth_hist.at<float>(i));
        int hist_value = (saturate_cast<int>(depth_hist.at<float>(i))) + 
                         (saturate_cast<int>(depth_hist.at<float>(i-1))) + 
                         (saturate_cast<int>(depth_hist.at<float>(i+1)));
        //output depth information
        //cout << "Number of Depth in " << i << " are " << hist_value_ori << endl;
        
        //compare
        if(hist_value >= first_value){
            if((i-1) != first_idx){
                second_idx = first_idx;
                second_value = first_value;
            }
            first_idx = i;
            first_value = hist_value;
        }else if(hist_value > second_value && (i-1) != first_idx){
            second_idx = i;
            second_value = hist_value;
        }
    }
    cout << "First in Depth: hist[" << first_idx << "] == " << first_value << endl;
    cout << "Second in Depth: hist[" << second_idx << "] == " << second_value << endl;
    //get Depth Index
    if(first_idx < second_idx){
        depth_idx = first_idx;
    }else{
        depth_idx = second_idx;
    }

    //Set start and end depth
    int depth_base = depth_idx;
    cube[4] = depth_idx - 15;
    cube[5] = depth_idx + 0;
    cout << "Depth_Base is: " << depth_base << "  Start_Depth: " << cube[4] << "  End_Depth: " << cube[5] << endl;

    //Handle Depth Frame ROI
    Mat depth_roi = depth_frame(hand_roi);
    depth_roi = depth_roi.clone();
    imshow("DEPTH ROI", depth_roi);
    
    //threshold the ROI
    threshold(depth_roi, depth_roi, (depth_idx-2), 255, THRESH_TOZERO);
    threshold(depth_roi, depth_roi, (depth_idx+2), 255, THRESH_TOZERO_INV);
    imshow("THRESH ROI", depth_roi);
    
    //store result of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //findContours
    getContours(depth_roi, contours, hierarchy);
    //get largest contour
    int largest_contour_idx = findLargestContourIndex(contours);
    //draw Contours
    Mat drawing = Mat::zeros(depth_roi.size(), CV_8UC3);
    drawContours(drawing, contours, hierarchy, CV_FILLED, largest_contour_idx, 10);
    imshow("DEPTH Contour", drawing);
    
    //get boundingRect
    Rect roi_rect = boundingRect(contours[largest_contour_idx]);
    cout << "ROI Rect -> Width: " << roi_rect.width << "  Height: " << roi_rect.height << endl;
    //Add Offset
    roi_rect.x = roi_rect.x + hand_roi.x;
    roi_rect.y = roi_rect.y + hand_roi.y;
    //Increase ROI Height
    roi_rect.height = (int)(roi_rect.height * 1.3);
    cout << "Increasea ROI Rect Height to " << roi_rect.height << endl;

    //Set Cube x and y
    cube[0] = roi_rect.x;
    cube[1] = roi_rect.y;
    cube[2] = roi_rect.x + roi_rect.width;
    cube[3] = roi_rect.y + roi_rect.height;
    //Check the Boundary
    if(cube[3] >= depth_frame.rows){
        cube[3] = depth_frame.rows;
    }

    //Show Cube result
    cout << "Cube ROI ->" << 
            " xmin:" << cube[0] << 
            " xmax:" << cube[1] << 
            " ymin:" << cube[2] << 
            " ymax:" << cube[3] <<
            " start_depth:" << cube[4] <<
            " end_depth:" << cube[5] << endl;
}

CubeGenerator::~CubeGenerator(){
}

