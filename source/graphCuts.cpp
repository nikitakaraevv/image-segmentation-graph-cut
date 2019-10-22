#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

#include "../maxflow/graph.h"

#include "image.h"

using namespace std;
using namespace cv;


int distance(Vec3b pixel1, Vec3b pixel2)
{
    double s = 30, k = 100;
    
    return int(k * exp(-pow(norm(pixel1, pixel2),2) / (2 * s*s)));
}

Image<Vec3b> imageToGraph(Image<Vec3b> Icolor, Vec3b fg_pixel, Vec3b bg_pixel)
{
    int n = Icolor.rows, m = Icolor.cols, mul_capacity = 0;
	Graph<int,int,int> g(/*estimated # of nodes*/ n*m, /*estimated # of edges*/ n*m*4);
	g.add_node(n*m);
	//g.add_tweights( 0,   /* capacities */  1, 5 );
	//g.add_tweights( 1,   /* capacities */  6, 1 );
    
    for (int i=1;i<n-1;i++){
        for (int j=1;j<m-1;j++){
            Vec3b pixel = Icolor.at<Vec3b>(i,j);
            int diff_right = distance(pixel, Icolor.at<Vec3b>(i,j+1)),
            diff_left = distance(pixel, Icolor.at<Vec3b>(i,j-1)),
            diff_bottom = distance(pixel, Icolor.at<Vec3b>(i+1,j)),
            diff_top = distance(pixel, Icolor.at<Vec3b>(i-1,j));
            
            mul_capacity = max(mul_capacity, max(max(diff_left, diff_right), max(diff_bottom, diff_top)));
            
            g.add_edge( m*i+j, m*i+j+1,    /* capacities */  diff_right, diff_right );
            g.add_edge( m*i+j, m*i+j-1,    /* capacities */  diff_left, diff_left );
            g.add_edge( m*i+j, m*(i+1)+j,    /* capacities */  diff_bottom, diff_bottom );
            g.add_edge( m*i+j, m*(i-1)+j,    /* capacities */  diff_top, diff_top );
            
            
        }
    }
    
    for (int i=0;i<n;i++){
        for (int j=0;j<m;j++){
            Vec3b pixel = Icolor.at<Vec3b>(i,j);
            double fg_prob = -log(norm(pixel, fg_pixel, NORM_L1) / (norm(pixel, fg_pixel, NORM_L1) + norm(pixel, bg_pixel, NORM_L1)));
            double bg_prob = -log(norm(pixel, bg_pixel, NORM_L1) / (norm(pixel, bg_pixel, NORM_L1) + norm(pixel, fg_pixel, NORM_L1)));
            
            double source_capacity = (fg_prob / (fg_prob + bg_prob));
            double sink_capacity = (bg_prob / (fg_prob + bg_prob));
        
            g.add_tweights( m*i+j,  /* capacities */  int(mul_capacity * source_capacity), int(mul_capacity * sink_capacity ));
        }
    }
	
	int flow = g.maxflow();
	cout << "Flow = " << flow << endl;
	for (int i=0;i<n;i++){
        for (int j=0;j<m;j++){
            if (g.what_segment(m*i+j) == Graph<int,int,int>::SINK)
                Icolor.at<Vec3b>(i,j) = Vec3b(0,0,0);
        }
    }
    return Icolor;
}

int main() {
	
	Image<Vec3b> Icolor = Image<Vec3b>(imread("../fishes.jpg"));
    
    int n = Icolor.rows, m = Icolor.cols;
    Vec3b fg_pixel = Icolor.at<Vec3b>(100,100);
    Vec3b bg_pixel = Icolor.at<Vec3b>(0,0);
    
    imageToGraph(Icolor, fg_pixel, bg_pixel);
    
    
    cout << "p1 = " << fg_pixel << " p2 = "<< bg_pixel << endl;
    cout << "diff = " << norm(fg_pixel, bg_pixel, NORM_L1) << endl;
    cout << "num of rows = " << n <<" num of cols = "<< m << endl;
	imshow("I", Icolor);
	waitKey(0);
	return 0;
}
