#include<iostream>
#include<vector>
#include<limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

const int width  = 400;
const int height = 400;


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

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main() {
        // int width =300,height = 300;

        TGAImage image(width, height, TGAImage::RGB);

        float *zbuffer = new float[width*height];
        for (int i=width*height; i>0 ;i--){
            zbuffer[i] = -std::numeric_limits<float>::max();
        }

        Model model("obj/african_head.obj");
        
        Vec3f light_dir(0,0,-1); // define light_dir

        for (int i=0; i<model.nfaces(); i++) {
            std::vector<int> face = model.face(i);
            Vec3f pts[3];
            for (int j=0; j<3; j++) {
                pts[j] = world2screen(model.vert(face[j]));
            }

            //这里是为了计算每个面的法向量
            //如果直接用pts[i]的话会导致intensity大部分都接近1，模型变成全白的
            Vec3f ver[3];
            for (int j=0; j<3; j++) {
                ver[j] = model.vert(face[j]);
            }
            Vec3f n=cross(ver[2]-ver[0],ver[1]-ver[0]);
            n.normalize(); 
            // std::cout <<n.x <<n.y <<n.z <<std::endl;
            float intensity = n * light_dir; 
            // std::cout <<intensity << std::endl;

            triangle(pts, zbuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        return 0;
}