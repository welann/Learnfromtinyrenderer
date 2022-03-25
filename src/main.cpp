#include "tgaimage.h"
#include "model.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

void line(Vec2i p1,Vec2i p2, TGAImage &image, TGAColor color) {
    bool steep = false;
    int x0=p1.x,y0=p1.y;
    int x1=p2.x,y1=p2.y;

    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    if (t0.y==t1.y && t0.y==t2.y) return; // I dont care about degenerate triangles 
    // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
    if (t0.y>t1.y) std::swap(t0, t1); 
    if (t0.y>t2.y) std::swap(t0, t2); 
    if (t1.y>t2.y) std::swap(t1, t2); 
    int total_height = t2.y-t0.y; 
    for (int i=0; i<total_height; i++) { 
        bool second_half = i>t1.y-t0.y || t1.y==t0.y; 
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y; 
        //这里一个alpha和beta代表了两种边，每种边的取样间隔
        //由于三角形是分段绘制的，所以会有一边是先画完的，这部分就是beta
        //而另一边需要等到另一半三角形画完才能画完，这部分就是alpha
        
        float alpha = (float)i/total_height; 
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here 
        Vec2i A =               t0 + (t2-t0)*alpha; 
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
        if (A.x>B.x) 
            std::swap(A, B); 

        for (int j=A.x; j<=B.x; j++) { 
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y 
        } 
    } 
}


int main(int argc, char** argv) {
        int width =300,height = 300;

        TGAImage image(width, height, TGAImage::RGB);

        Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
        Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
        Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 

        triangle(t0[0], t0[1], t0[2], image, red); 
        triangle(t1[0], t1[1], t1[2], image, white); 
        triangle(t2[0], t2[1], t2[2], image, green);

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        return 0;
}