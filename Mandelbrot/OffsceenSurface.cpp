/*
 * Bitmap surface primitives and palette handling classes
 *
 * Copyright (c) 2015 Nikita Shulga
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "OffsceenSurface.h"
#include <random>
#include <assert.h>
#include <fstream>

void Palette::save(const std::string &name)
{
    std::ofstream outFile;
    outFile.open (name);
    outFile<<data.size()<<std::endl;
    for (unsigned i(0); i < data.size(); ++i)
        outFile<<int(data[i].getR())<<", "<<int(data[i].getG())<<", "<<int(data[i].getB())<<std::endl;
    outFile.close();
}

void Palette::load(const std::string &name)
{
    std::ifstream inFile;
    inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    inFile.open(name);
    unsigned palSize;
    inFile>>palSize;
    data.resize(palSize);
    for(unsigned i(0);i<palSize;++i) {
        unsigned r,g,b;
        char comma;
        inFile>>r>>comma>>g>>comma>>b;
        data[i]=RGB<unsigned char>(r,g,b);
    }
    inFile.close();
}

void Palette::randomize()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0,255*255*255);
    for(unsigned i(0); i < data.size(); ++i) {
        auto val = dis(gen);
        data[i] = RGB<unsigned char>((val>>16)&255,(val>>8)&255,val&255);
    }
}


template<typename T> T sqr(T x) { return x*x; }

void Palette::randomizeUV(unsigned char Y, unsigned distance)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0,225*225);
    unsigned i = 0;
    auto pu = 0, pv = 0;
    while(i < data.size())
    {
        auto val = dis(gen);
        auto u = 16+val%225;
        auto v = 16+val/225;
        if (sqr(pu-u)+sqr(pv-v) < sqr(distance)) continue;
        pu=u; pv=v;
        auto r = Y+1.370705*(v-128);
        auto g = Y-.698001* (v-128) - 0.337633*(u-128);
        auto b = Y+1.732446*(u-128);
        data[i++]=RGB<unsigned char>(r,g,b);
    }
}

RGB<unsigned char >& Palette::operator[](size_t idx)
{
    assert (idx < data.size());
    return data[idx];
}


OffscreenSurface::OffscreenSurface(unsigned w, unsigned h):width(w),height(h)
{
    rgb = new unsigned char[width*height*3];
}

OffscreenSurface::OffscreenSurface(unsigned w, unsigned h, Palette &p):width(w),height(h),palette(p)
{
    rgb = new unsigned char[width*height*3];
}


void OffscreenSurface::putPixel(unsigned x, unsigned y, unsigned char r, unsigned char g, unsigned char b)
{
    auto offs = (y*width+x)*3;
    rgb[offs+0] = r;
    rgb[offs+1] = g;
    rgb[offs+2] = b;
}

void OffscreenSurface::putPixel(unsigned x, unsigned y, RGB<unsigned char> c)
{
    putPixel(x,y,c.getR(), c.getG(), c.getB());
}

void OffscreenSurface::putPixel(unsigned x, unsigned y, unsigned idx)
{
    putPixel(x,y, palette[idx]);
}

void OffscreenSurface::putPixel(unsigned x, unsigned y, float val)
{
    val *= palette.size();
    unsigned idx = unsigned(floor(val));
    if (idx >= palette.size()-1) {
        putPixel(x,y, palette[palette.size()-1]);
        return;
    }
    float a = val - idx;
    putPixel(x,y, (1-a)*palette[idx]+a*palette[idx+1]);
}


OffscreenSurface::~OffscreenSurface() {
    delete[] rgb;
}


void OffscreenSurface::clear()
{
    memset(rgb, 0, 3*width*height);
}

#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>
#include <CoreServices/CoreServices.h>

static bool saveImageToPNG(const char *fName, CGImageRef image)
{
    CFStringRef cfName = CFStringCreateWithCString(NULL, fName, kCFStringEncodingASCII);
    CFURLRef cfUrl = CFURLCreateWithFileSystemPath(NULL, cfName, kCFURLPOSIXPathStyle, false);
    CGImageDestinationRef destImage = CGImageDestinationCreateWithURL(cfUrl, kUTTypePNG , 1, NULL);
    CGImageDestinationAddImage(destImage, image, NULL);
    bool rc = CGImageDestinationFinalize(destImage);
    CFRelease(destImage);
    CFRelease(cfUrl);
    CFRelease(cfName);
    return rc;
}

static bool saveImageToJPEG(const char *fName, CGImageRef image)
{
    CFStringRef cfName = CFStringCreateWithCString(NULL, fName, kCFStringEncodingASCII);
    CFURLRef cfUrl = CFURLCreateWithFileSystemPath(NULL, cfName, kCFURLPOSIXPathStyle, false);
    CGImageDestinationRef destImage = CGImageDestinationCreateWithURL(cfUrl, kUTTypeJPEG , 1, NULL);
    CGImageDestinationAddImage(destImage, image, NULL);
    bool rc = CGImageDestinationFinalize(destImage);
    CFRelease(destImage);
    CFRelease(cfUrl);
    CFRelease(cfName);
    return rc;
}


void OffscreenSurface::saveToPNG(const std::string &name)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef data = CGDataProviderCreateWithData(NULL, rgb, 3*width*height, NULL);
    CGImageRef imageRef = CGImageCreate(width, height, 8, 24, 3*width, colorSpace, kCGBitmapByteOrderDefault, data, NULL, false, kCGRenderingIntentDefault);
    saveImageToPNG(name.data(), imageRef);
    CGDataProviderRelease(data);
    CGColorSpaceRelease(colorSpace);
    CGImageRelease(imageRef);
    CFRelease(imageRef);
}

void OffscreenSurface::saveToJPEG(const std::string &name)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef data = CGDataProviderCreateWithData(NULL, rgb, 3*width*height, NULL);
    CGImageRef imageRef = CGImageCreate(width, height, 8, 24, 3*width, colorSpace, kCGBitmapByteOrderDefault, data, NULL, false, kCGRenderingIntentDefault);
    saveImageToJPEG(name.data(), imageRef);
    CGDataProviderRelease(data);
    CGColorSpaceRelease(colorSpace);
    CGImageRelease(imageRef);
    CFRelease(imageRef);
}