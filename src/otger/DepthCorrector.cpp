
#include "DepthCorrector.h"


void DepthCorrectorStream::correctDepthFrameA(const DepthFrame& source, DepthFrame& target)
{
    target = source;
    DepthPixel* pixels = target.pixels;
    for (int c = 0; c < DepthFrame::HEIGHT * DepthFrame::WIDTH; ++c) {
        unsigned short depth = pixels[c].depth;

        if (depth == 0) {
            int col = c % DepthFrame::WIDTH;
            int row = c / DepthFrame::WIDTH;

            if (col != 0) {
                depth = pixels[c - 1].depth;
                if (row != 0) {
                    depth = (depth + pixels[col + (row - 1) * DepthFrame::WIDTH].depth) / 2;
                }
            }
            else if (row != 0) {
                depth = pixels[col + (row - 1) * DepthFrame::WIDTH].depth;
            }
        }

        depth = (depth < MIN_DEPTH && depth != 0) ? MIN_DEPTH : depth;
        depth = (depth >= MAX_DEPTH) ? MAX_DEPTH - 1 : depth;
        pixels[c].depth = depth;
    }
}

void DepthCorrectorStream::correctDepthFrameB(const DepthFrame& source, DepthFrame& target)
{
    static DepthFrame* memoryFrame = nullptr;
    if (memoryFrame == nullptr) {
        memoryFrame = new DepthFrame();
        memoryFrame->clear();
    }

    target = source;
    DepthPixel* pixels = target.pixels;
    DepthPixel* memory = memoryFrame->pixels;
    for (int c = 0; c < DepthFrame::HEIGHT * DepthFrame::WIDTH; ++c) {
        unsigned short depth = pixels[c].depth;
        depth = (depth < MIN_DEPTH && depth != 0) ? MIN_DEPTH : depth;
        depth = (depth >= MAX_DEPTH) ? MAX_DEPTH - 1 : depth;

        if (depth == 0) depth = memory[c].depth;
        else {
            if (memory[c].depth != 0) {
                int diff = abs(memory[c].depth - depth);
                if (diff < 100) memory[c].depth = (unsigned short)(memory[c].depth * 0.2f + depth * 0.8f);
                else memory[c].depth = depth;
            }
            else memory[c].depth = depth;
        }

        pixels[c].depth = depth;
    }
}

void DepthCorrectorStream::correctDepthFrameC(const DepthFrame& source, DepthFrame& target)
{
    const DepthPixel* src = source.pixels;
    DepthPixel* dest = target.pixels;
    for (int c = 0; c < DepthFrame::HEIGHT * DepthFrame::WIDTH; ++c) {
        dest[c].playerIndex = src[c].playerIndex;

        unsigned short depth = src[c].depth;
        if (depth == 0) {
            int col = c % DepthFrame::WIDTH;
            int row = c / DepthFrame::WIDTH;

            struct {
                unsigned short depth;
                short count;
            } freqs[24];

            int freqsN = 0;

            int inner = 0, outer = 0;

            for (int dx = -2; dx <= 2; ++dx) {
                for (int dy = -2; dy <= 2; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    int x = col + dx;
                    int y = row + dy;
                    if (x < 0 || x >= DepthFrame::WIDTH || y < 0 || y >= DepthFrame::HEIGHT) continue;

                    int idx = x + y*DepthFrame::WIDTH;
                    if (src[idx].depth != 0) {
                        unsigned short depth = src[idx].depth;
                        int i;
                        for (i = 0; i < freqsN; ++i) {
                            if (freqs[i].depth == depth) {
                                ++freqs[i].count;
                                break;
                            }
                        }
                        if (i == freqsN) {
                            freqs[freqsN].depth = depth;
                            freqs[freqsN++].count = 1;
                        }

                        i = int(x > -2 && y > -2 && x < 2 && y < 2);
                        inner += i;
                        outer += (1 - i);
                    }
                }
            }

            depth = 0;

            if (inner >= 2 || outer >= 2) {
                short count = 0;
                for (int i = 0; i < freqsN; ++i) {
                    if (freqs[i].count > count) {
                        count = freqs[i].count;
                        depth = freqs[i].depth;
                    }
                }
            }
        }

        depth = (depth < MIN_DEPTH && depth != 0) ? MIN_DEPTH : depth;
        depth = (depth >= MAX_DEPTH) ? MAX_DEPTH - 1 : depth;
        dest[c].depth = depth;
    }
}


void DepthCorrectorStream::correctDepthFrame(const DepthFrame& source, DepthFrame& target)
{
    correctDepthFrameB(source, target);
}
