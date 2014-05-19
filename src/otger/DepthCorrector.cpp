
#include "DepthCorrector.h"


void DepthCorrectorStream::correctDepthFrameA(const DataStream::DepthPixel* source, DataStream::DepthPixel* target)
{
    memcpy(target, source, DEPTH_FRAME_SIZE);
    for (int c = 0; c < DEPTH_FRAME_HEIGHT * DEPTH_FRAME_WIDTH; ++c) {
        unsigned short depth = target[c].depth;

        if (depth == 0) {
            int col = c % DEPTH_FRAME_WIDTH;
            int row = c / DEPTH_FRAME_WIDTH;

            if (col != 0) {
                depth = target[c - 1].depth;
                if (row != 0) {
                    depth = (depth + target[col + (row - 1) * DEPTH_FRAME_WIDTH].depth) / 2;
                }
            }
            else if (row != 0) {
                depth = target[col + (row - 1) * DEPTH_FRAME_WIDTH].depth;
            }
        }

        depth = (depth < MIN_DEPTH && depth != 0) ? MIN_DEPTH : depth;
        depth = (depth >= MAX_DEPTH) ? MAX_DEPTH - 1 : depth;
        target[c].depth = depth;
    }
}

void DepthCorrectorStream::correctDepthFrameB(const DataStream::DepthPixel* source, DataStream::DepthPixel* target)
{
    static DataStream::DepthPixel* memory = nullptr;
    if (memory == nullptr) {
        memory = newDepthFrame();
        memset(memory, 0, DEPTH_FRAME_SIZE);
    }

    memcpy(target, source, DEPTH_FRAME_SIZE);
    for (int c = 0; c < DEPTH_FRAME_HEIGHT * DEPTH_FRAME_WIDTH; ++c) {
        unsigned short depth = target[c].depth;
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

        target[c].depth = depth;
    }
}

void DepthCorrectorStream::correctDepthFrameC(const DataStream::DepthPixel* source, DataStream::DepthPixel* target)
{
    for (int c = 0; c < DEPTH_FRAME_HEIGHT * DEPTH_FRAME_WIDTH; ++c) {
        target[c].playerIndex = source[c].playerIndex;

        unsigned short depth = source[c].depth;
        if (depth == 0) {
            int col = c % DEPTH_FRAME_WIDTH;
            int row = c / DEPTH_FRAME_WIDTH;

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
                    if (x < 0 || x >= DEPTH_FRAME_WIDTH || y < 0 || y >= DEPTH_FRAME_HEIGHT) continue;

                    int idx = x + y*DEPTH_FRAME_WIDTH;
                    if (source[idx].depth != 0) {
                        unsigned short depth = source[idx].depth;
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
        target[c].depth = depth;
    }
}


void DepthCorrectorStream::correctDepthFrame(const DataStream::DepthPixel* source, DataStream::DepthPixel* target)
{
    correctDepthFrameB(source, target);
}
