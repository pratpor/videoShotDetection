///////////////////Headers////////////////////////
#include <iostream>
#include <cv.h>
#include <highgui.h>
using namespace std;
using namespace cv;

int main()
{
    ///////////////////Taking Video File in Input
    cout<<"Enter the File path of the AVI video File:\n";
    char filename[100],filename2[100];
    cin>>filename;
    VideoCapture input(filename);
    if(!input.isOpened())
    {
        cout<<"Error Opening video File";
        return -1;
    }
    else
    cout<<"File selected = "<<filename<<endl;


    ////////////////Generating Summary Video Names
    strcpy(filename2,filename);
    strcat(filename,"_Summary.avi");
    strcat(filename2,"Summary_Static.avi");
    cout<<filename<<" "<<filename2<<endl;



    ////////////Getting properties of the input Video
    int v_height = input.get(CV_CAP_PROP_FRAME_HEIGHT);
    int v_width = input.get(CV_CAP_PROP_FRAME_WIDTH);
    cout<<v_height<<" "<<v_width<<endl;


    ///////////Declaring frames to store images
    Mat bgr_frame1,bgr_frame2,hsv_frame1,hsv_frame2,dst;
    input>>bgr_frame1; //input frame from video
    namedWindow("Video", CV_WINDOW_AUTOSIZE); //window where to show the output



    /// Declaring parameters for histogram calculation and difference
    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };
    int fps = input.get(CV_CAP_PROP_FPS);
    int ex = static_cast<int>(input.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int) input.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) input.get(CV_CAP_PROP_FRAME_HEIGHT));

    // hue varies from 0 to 256, saturation from 0 to 180
    float h_ranges[] = { 0, 256 };
    float s_ranges[] = { 0, 180 };
    dst.create(v_height,v_width,ex);    //Dst object creation for zoom out effect

    const float* ranges[] = { h_ranges, s_ranges };

    // Use the o-th and 1-st channels
    int channels[] = { 0, 1 };
    MatND hist_frame1;
    MatND hist_frame2;
    int count=1;
    int newshot=2*fps, sum = 3*fps;


    ///////////////////Variables to store time
    int hours=0,mins=0,secs=0;



    ////////////Creating object for writing frames to video
    VideoWriter summary;
    summary.open(filename,ex,fps,S,true); //moving image summary object
    if(!summary.isOpened())
    {
        cout<<"\nProblem in opening output file. Video Summarization wont take place but Shot detection Will Work\n";
    }
    VideoWriter summary1;
    summary1.open(filename2,ex,fps,S,true); //still image summary object
    if(!summary1.isOpened())
    {
        cout<<"\nProblem in opening output file. Video Summarization wont take place but Shot detection Will Work\n";
    }


    cout<<"\n\n";
    cout<<"Shot No."<<count<<" started at time "<<hours<<" : "<<mins<<" : "<<secs<<"\n";
    cout<<"Taking summary from here\n";


    //extracting and processing frames
    int fcount=0;
    while(true)
    {

        fcount++;
        if(bgr_frame1.empty())
        break;
        if(newshot>0)
        newshot--;
        if(sum>0)
        {
            sum--;
            summary<<bgr_frame1; //writing frame to moving summary object
        }

        //////////keeping trach of time
        if(fcount>fps)
        {
            secs++;
            if(secs>=60)
            {
                secs=0;
                mins++;
            }
            if(mins>=60)
            {
                mins=0;
                hours++;
            }
            fcount=0;
        }
        input>>bgr_frame2;


        ////////BGR to HSV conversion
        cvtColor(bgr_frame1,hsv_frame1,CV_BGR2HSV);
        cvtColor(bgr_frame2,hsv_frame2,CV_BGR2HSV);


        //////////Histogram Calculation
        calcHist( &hsv_frame1, 1, channels, Mat(), hist_frame1, 2, histSize, ranges, true, false );
        normalize( hist_frame1, hist_frame1, 0, 1, NORM_MINMAX, -1, Mat() );

        calcHist( &hsv_frame2, 1, channels, Mat(), hist_frame2, 2, histSize, ranges, true, false );
        normalize( hist_frame2, hist_frame2, 0, 1, NORM_MINMAX, -1, Mat() );



        ////////////Histogram Difference Calculation
        double hist_diff = compareHist( hist_frame1, hist_frame2, 3 );


        //////threshold condition checking
        if(hist_diff > 0.3 && newshot==0)
        {
            count++;
            cout<<"Shot No."<<count<<" started at time "<<hours<<" : "<<mins<<" : "<<secs<<"\n";
            int i=50;
            int x1=50,y1=50,x2=v_width-50,y2=v_height-50;
            while(i--)
            {
                Mat zoom(bgr_frame1,Rect(x1--,y1--,x2++,y2++));
                resize(zoom,zoom,dst.size());
                summary1<<zoom; // writing frame to static summary object
                cvWaitKey(60);
            }
            newshot=2*fps;
        }

        /////////threshold checking for taking summary
        if(hist_diff > 0.45 && sum==0)
        {
            //summary <<bgr_frame1;
            cout<<"\nTaking summary from here\n";
            sum = 4*fps;
        }


        ///////output image being processed
        imshow("Video",bgr_frame1);
        bgr_frame1 = bgr_frame2.clone();

        if(waitKey(30) >= 0)
        continue;

    }

    ///////print the total number of shots in the video
    cout<<"Total number of shots in given video = "<<count-1<<endl;

    return 0;
}
