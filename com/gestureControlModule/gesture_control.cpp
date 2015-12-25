//Gesture Controller
#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "gesture_control.hpp"
#include "contours_operator.hpp"

#include <signal.h>
#include <syslog.h>

#ifdef ENABLE_INPUT_SERVICE
extern "C" {
#include "input_inf.h"
};
#endif

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;

GestureControl::GestureControl(){
    
    //initial syslog
    openlog("gesture-aue", LOG_PID | LOG_CONS, LOG_USER);

#ifdef ENABLE_INPUT_SERVICE
    //initial InputService
    if( initInterface(input_type) != SUCCESS ){
        cout << "Error!! initInterface failure!" << endl;
        syslog(LOG_ERR, "Gesture -> do inputservice initInterface failure!");
        exit(0);
    }
#endif

}

void GestureControl::setGestureROI(int xmin, int ymin, int xmax, int ymax, int depth_min, int depth_max){
    
    //ROI for tracking
    roi_rect = Rect(xmin, ymin, (xmax - xmin), (ymax - ymin));
    cout << "ROI Rect -> Width: " << roi_rect.width << "  Height: " << roi_rect.height << endl;

    //Set Depth range
    start_depth = depth_min;
    end_depth = depth_max;
}

#if 0
void GestureControl::generateStaticGestureROI(){
    //Calculate ROI parameter
    int source_image_width = capture.get(3);
    int source_image_height = capture.get(4);
    cout << "Frame Width: " << source_image_width << endl;
    cout << "Frame Height: " << source_image_height << endl;

    //ROI Rect
    int roi_rect_width = source_image_width * 0.5;
    int roi_rect_height = source_image_height * 0.4;
    int roi_rect_start_x = (source_image_width - roi_rect_width) * 0.5 ;
    int roi_rect_start_y = (source_image_height - roi_rect_height) * 0.5;
    
    //ROI for tracking
    roi_rect = Rect(roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);
    cout << "ROI Rect -> Width: " << roi_rect.width << "  Height: " << roi_rect.height << endl;

    //Set Depth range
    start_depth = 65;
    end_depth = 80;
}

void GestureControl::generateGestureROI(const Rect &hand_roi, const Mat &depth_frame){
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
        int hist_value = (saturate_cast<int>(depth_hist.at<float>(i))) + (saturate_cast<int>(depth_hist.at<float>(i-1))) + (saturate_cast<int>(depth_hist.at<float>(i+1)));
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

    //Handle Depth Frame ROI
    Mat depth_roi = depth_frame(hand_roi);
    depth_roi = depth_roi.clone();
    imshow("DEPTH ROI", depth_roi);
    
    //threshold the ROI
    threshold(depth_roi, depth_roi, (depth_idx-2), 255, THRESH_TOZERO);
    threshold(depth_roi, depth_roi, (depth_idx+2), 255, THRESH_TOZERO_INV);
    imshow("THRESH ROI", depth_roi);
    //Mat depth_roi_inv;
    //bitwise_not(depth_roi, depth_roi_inv);
    //imshow("BITWISE_NOT THRESH ROI", depth_roi_inv);

    //Set start and end depth
    depth_base = depth_idx;
    start_depth = depth_idx - 15;
    end_depth = depth_idx + 0;
    cout << "Depth_Base is: " << depth_base << "  Start_Depth: " << start_depth << "  End_Depth: " << end_depth << endl;
    
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
    roi_rect = boundingRect(contours[largest_contour_idx]);
    cout << "ROI Rect -> Width: " << roi_rect.width << "  Height: " << roi_rect.height << endl;
    //Add Offset
    roi_rect.x = roi_rect.x + hand_roi.x;
    roi_rect.y = roi_rect.y + hand_roi.y;
    //Increase ROI Height
    roi_rect.height = (int)(roi_rect.height * 1.3);
    cout << "Increasea ROI Rect Height to " << roi_rect.height << endl;
    

    /*
    //draw Contours
    Mat drawing = Mat::zeros(depth_roi.size(), CV_8UC3);
    for( size_t i = 0; i < contours.size(); i++ )
    {
        if((contourArea((contours)[i]) <= 10)){
            continue;
        }

        Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
        drawContours( drawing, contours, int(i), color, 1, 8, hierarchy, 0, Point() );
    }
    imshow("DEPTH Contour", drawing);
    */

    /*
    int hist_w = 500;
    int hist_h = 800;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar(0,0,0) );
    //cout << "Hist.rows: " << histImage.rows << "  Hist.cols: " << histImage.cols << endl;
    normalize(depth_hist, depth_hist, 0, histImage.rows, NORM_MINMAX);
    //show Hist
    //cout << "Depth Hist -> rows: " << depth_hist.rows << "  cols: " << depth_hist.cols << "  channels: " << depth_hist.channels() << endl;

    for(int i = 0; i < histSize; i++){
        int val = saturate_cast<int>(depth_hist.at<float>(i)*histImage.rows/255);
        
        Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
        rectangle(histImage,
                Point( bin_w*(i), histImage.rows ),
                Point( bin_w*(i+1), histImage.rows - val ),
                color, -1, 8);
    }
    imshow("Histogram", histImage);
    */
    
}
#endif

void GestureControl::trackingObjectByContour(int largest_contour_index){
    
    //Using Contour tracking Object
    if(largest_contour_index != -1)
    {
        //depth image exist
        //drawContours(&drawing, &contours, &hierarchy, CV_FILLED, largest_contour_index, area_threshold);

        //get TrackBox, TrackBox is boundingRectangle of the largest contour
        trackBox = boundingRect(contours[largest_contour_index]);
        //center point of trackBox
        center_of_trackBox = Point( (trackBox.x + (trackBox.width/2)), (trackBox.y + (trackBox.height/2)) );

        /**
            if origin == resetPoint and center_of_trackBox is inside roi rectangle
                set center_of_trackBox as origin point and current point
            else if origin is setted and center_of_trackBox is inside roi rectangle
                set center_of trackBox as current point
        **/
        if(origin == resetPoint && roi_rect.contains(center_of_trackBox))
        {
            origin = center_of_trackBox;
            current = origin;
            //cout << "Entry Point -> origin( " << origin.x << ", " << origin.y << ")" << endl;
        }else if(roi_rect.contains(center_of_trackBox)){
            current = center_of_trackBox;
        }

        //Draw center of trackBox
        circle( drawing, center_of_trackBox, 5, Scalar(181,186,10), -1, 8, 0 );
        circle( debug_frame, center_of_trackBox, 5, Scalar(181,186,10), -1, 8, 0 );

    }
}

void GestureControl::resetGestureEnviroment(){
    origin = resetPoint;
    current = resetPoint;
    gesture_command = NONE;
}

void GestureControl::getGestureDirection(){
    //value of tantheta
    float tantheta = (float)(abs(current.y - origin.y))/(float)(abs(current.x - origin.x));

    //judgement for gesture
    if(tantheta <= sqrt(3))
    {
        //Left and Right
        if((current.x - origin.x) >= threshold_of_displacement)
        {
            //Left
            gesture_command = LEFT;
        }else{
            //Right
            gesture_command = RIGHT;
        }
    }else{
        //Up and Down
        if((current.y - origin.y) >= threshold_of_displacement)
        {
            //Down
            gesture_command = DOWN;
        }else{
            //Up
            gesture_command = UP;
        }
    }
}

void GestureControl::sendGestureCommand(){
    //Trigger Command
    switch(gesture_command){
        case RIGHT:
            //Trigger Right Command
            cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
            syslog(LOG_INFO, "Trigger RIGHT RIGHT RIGHT RIGHT !!!!");
#ifdef ENABLE_INPUT_SERVICE
            //Do InputService key command
            sendMsg(KEY_MSG, "Right");
            syslog(LOG_INFO, "Gesture sendMsg(Right) to inputservice !");
#endif
            break;
        case LEFT:
            //Trigger Left Command
            cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
            syslog(LOG_INFO, "Trigger LEFT LEFT LEFT LEFT !!!!");
#ifdef ENABLE_INPUT_SERVICE
            //Do InputService key command
            sendMsg(KEY_MSG, "Left");
            syslog(LOG_INFO, "Gesture snedMsg(Left) to inputservice !");
#endif
            break;
        case UP:    
            //Trigger Up Command
            cout << "Trigger UP UP UP UP !!!!" << endl;
            syslog(LOG_INFO, "Trigger UP UP UP UP !!!!");
#ifdef ENABLE_INPUT_SERVICE
            //Do InputService key command
            sendMsg(KEY_MSG, "Up");
            syslog(LOG_INFO, "Gesture snedMsg(Up) to inputservice !");
#endif
            break;
        case DOWN:
            //Trigger Down Command
            cout << "Trigger DOWN DOWN DOWN DOWN !!!!" << endl;
            syslog(LOG_INFO, "Trigger DOWN DOWN DOWN DOWN !!!!");
#ifdef ENABLE_INPUT_SERVICE
            //Do InputService key command
            sendMsg(KEY_MSG, "Down");
            syslog(LOG_INFO, "Gesture snedMsg(Down) to inputservice !");
#endif
            break;
    }
}

void GestureControl::filterDepthImage(Mat &src_image, Mat &dst_image, int start_depth, int end_depth){
	
	//Joe add threshold
	threshold(src_image, dst_image, start_depth, 255, THRESH_TOZERO);
	threshold(dst_image, dst_image, end_depth, 255, THRESH_TOZERO_INV);
	
	//adaptiveThreshold(dst_image, dst_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

	//imshow( "threshold depth map", dst_image );
}

void GestureControl::processNextFrame(Mat &depth_frame){
    
    //filterDepthImage
    //cout << "filter depth with " << start_depth << " and " << end_depth << endl;
    filterDepthImage(depth_frame, depth_frame, start_depth, end_depth);
    //show depth image
    //imshow("Filtered Depth", depth_frame);
    //Create ROI Mask
    mask = Mat(depth_frame.size(), CV_8UC1, Scalar::all(255));
    mask(roi_rect).setTo(Scalar::all(0));
    //imshow("Mask", mask);
    depth_frame.setTo( 0, mask);
    //imshow("Depth with Mask", depth_frame);

    //Create debug frame
    debug_frame = Mat(depth_frame.size(), CV_8UC3, Scalar::all(255));
    depth_frame.copyTo(debug_frame);

    //clear drawing
    if(origin == resetPoint){
        drawing = Mat::zeros(depth_frame.size(), CV_8UC3);
    }
    
    //findContours
    getContours(depth_frame, contours, hierarchy);
    //get largest contour
    largest_contour_index = findLargestContourIndex(contours);
    //cout << "largest_contour_index: " << largest_contour_index << endl;
    
    //get tracking object
    trackingObjectByContour(largest_contour_index);
    
    /**
        if 
            1. origin point and current point are setted
            2. origin point and current point "are not" the same point
            3. can not find any contour in current frame
        then
            1. calculate value of tantheta
            2. judge gesture command
            3. after judgement, reset gesture judgement environment variable
    **/
    if( origin != resetPoint
            && current != resetPoint 
            && largest_contour_index == -1 
            && origin != current )
    {
        //cout << "Destination Point -> destination( " << current.x << ", " << current.y << ")" << endl;

        //getGestureDirection
        getGestureDirection();

        //sendGestureCommand
        sendGestureCommand();
        
        //reset environment variable
        resetGestureEnviroment();
    }
    /**
        this is a special case: there is only one point, gesture judgement can not be done !!
        if 
            1. origin point and current point are setted
            2. origin point and current point "are" the same point
            3. can not find any contour in current frame
        then
            1. drop this gesture judgement, reset gesture environment variable
    **/
    else if(origin != resetPoint 
            && current != resetPoint 
            && origin == current 
            && largest_contour_index == -1 ){
        //reset environment variable
        resetGestureEnviroment();
    }

} 

void GestureControl::shutdownGesture(){
#ifdef ENABLE_INPUT_SERVICE
	//shutdown InputService Interface
	shutdownInterface();
#endif
	//close syslog
	closelog();
}

