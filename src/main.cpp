#include<iostream>
#include<vector>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


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

Vec3f barycentric(Vec2<int> pts[], Vec2i P) { 
    //原始的代码有点问题，只能这样修改
    //Vec3f u = Vec3f(pts[2][0]-pts[0][0], pts[1][0]-pts[0][0], pts[0][0]-P[0])^Vec3f(pts[2][1]-pts[0][1], pts[1][1]-pts[0][1], pts[0][1]-P[1]);
    Vec3f u = Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y);

    /* `pts` and `P` has integer value as coordinates
    so `abs(u[2])` < 1 means `u[2]` is 0, that means
    triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.0-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
} 

void triangle(Vec2i *pts, TGAImage &image, TGAColor color) { 
    Vec2i bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2i bboxmax(0, 0); 
    Vec2i clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    } 
    Vec2i P; 
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            image.set(P.x, P.y, color); 
        } 
    } 
} 


int main(int argc, char** argv) {
        int width =300,height = 300;

        TGAImage image(width, height, TGAImage::RGB);

        // Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
        // triangle(pts, image, red); 
        Model model("obj/african_head.obj");
        
        Vec3f light_dir(0,0,-1); // define light_dir

        for (int i=0; i<model.nfaces(); i++) { 
            std::vector<int> face = model.face(i); 
            Vec2i screen_coords[3]; 
            Vec3f world_coords[3]; 
            for (int j=0; j<3; j++) { 
                Vec3f v = model.vert(face[j]); 
                screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
                world_coords[j]  = v; 
            } 
            Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
            n.normalize(); 
            float intensity = n*light_dir; 
            if (intensity>0) { 
                triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
            } 
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        return 0;
}