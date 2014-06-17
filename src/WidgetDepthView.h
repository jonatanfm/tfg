#ifndef WIDGETDEPTHVIEW_H
#define WIDGETDEPTHVIEW_H

#pragma once

#include "globals.h"

#include "RenderUtils.h"
#include "DataStream.h"
#include "WidgetOpenGL.h"

// Widget renderer that shows the depth frames provided by a stream.
class WidgetDepthView : public RendererOpenGL
{
    private:
        Ptr<DataStream> stream;
        Texture texture;

        FrameNum frameNum;
        ALIGN(16) DepthFrame frame;

        // Buffer to update the texture

        ALIGN(16) unsigned char textureBuffer[DepthFrame::SIZE * 4];

    public:
        WidgetDepthView(Ptr<DataStream> stream) :
            stream(stream)
        {
            setAspect(DepthFrame::WIDTH, DepthFrame::HEIGHT);
        }

        ~WidgetDepthView()
        {
            if (stream) stream->removeNewFrameCallback(this);
        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        void initialize() override
        {
            texture = RenderUtils::createTexture(DepthFrame::WIDTH, DepthFrame::HEIGHT);

            stream->addNewFrameCallback(this, [this](const ColorFrame*, const DepthFrame*, const SkeletonFrame*) -> void {
                emit this->triggerRefresh();
            });
        }

        // Convert the depth frame to a RGBA image for visualization
        void frameToImage()
        {
            const DepthPixel* src = frame.pixels;
            const DepthPixel* end = frame.pixels + DepthFrame::SIZE;

            unsigned char* dest = textureBuffer;

            #ifndef NOT_VECTORIZED
                #define SRC(i) static_cast<short>(static_cast<unsigned short>((src + i)->depth))
                #define IDX(i) static_cast<short>(static_cast<unsigned short>((src + i)->playerIndex))

                // Vectorized assuming DepthFrame::SIZE % 16 == 0
                __m128i min = _mm_set1_epi16(static_cast<short>(DepthFrame::MIN_DEPTH));
                __m128i max = _mm_set1_epi16(static_cast<short>(DepthFrame::MAX_DEPTH) - 1);
                __m128i _0 = _mm_setzero_si128();
                __m128i _1 = _mm_cmpeq_epi32(_0, _0);
                __m128i _div25 = _mm_set1_epi16(18351);
                __m128i shuffleLo = _mm_set_epi8(-1, 6, 6, 6, -1, 4, 4, 4, -1, 2, 2, 2, -1, 0, 0, 0);
                __m128i shuffleHi = _mm_set_epi8(-1, 14, 14, 14, -1, 12, 12, 12, -1, 10, 10, 10, -1, 8, 8, 8);
                __m128i colorInvalid = _mm_set1_epi32(0x000000FF);
                __m128i colorPlayer = _mm_set1_epi32(0x0000FF00);
                for (; src < end; src += 8, dest += 8 * 4) {
                    __m128i d = _mm_set_epi16(SRC(7), SRC(6), SRC(5), SRC(4), SRC(3), SRC(2), SRC(1), SRC(0));
                    __m128i idxs = _mm_set_epi16(IDX(7), IDX(6), IDX(5), IDX(4), IDX(3), IDX(2), IDX(1), IDX(0));

                    // Invalid := ((d < min) || (d > max))  [or]  (d == 0)
                    // __m128i invalid = _mm_or_si128(_mm_cmplt_epi16(d, min), _mm_cmpgt_epi16(d, max));
                    __m128i invalid = _mm_cmpeq_epi16(d, _0);

                    // d := MAX(MIN(max, d) - min, 0)
                    d = _mm_subs_epu16(_mm_min_epu16(max, d), min);

                    // Transform to [0, 255]:  d := (2*d)/25   [d := (d * 256) / (4000 - 800)]
                    d = _mm_add_epi16(d, d);
                    { // Divide by 25
                        __m128i t = _mm_mulhi_epu16(d, _div25);
                        d = _mm_srli_epi16(_mm_add_epi16(_mm_srli_epi16(_mm_sub_epi16(d, t), 1), t), 4);
                    }

                    // d := 255 - d
                    d = _mm_xor_si128(d, _1);

                    // Extract the lower bytes from each short into two vectors, and make the RGBA pixels
                    #define MAKE_RGBA(shuffleMask, offset) { \
                            __m128i c = _mm_shuffle_epi8(d, shuffleMask); \
                            __m128i notPlayer = _mm_cmpeq_epi8(_0, _mm_shuffle_epi8(idxs, shuffleMask)); \
                            c = _mm_blendv_epi8(c, colorInvalid, _mm_shuffle_epi8(invalid, shuffleMask)); \
                            c = _mm_andnot_si128(_mm_andnot_si128(notPlayer, colorPlayer), c); \
                            _mm_store_si128((__m128i*)dest + offset, c); \
                        }

                    MAKE_RGBA(shuffleLo, 0);
                    MAKE_RGBA(shuffleHi, 1);

                    #undef MAKE_RGBA
                }
            #else
                while (src < end) {
                    USHORT depth = src->depth;

                    int valid = int(depth >= DepthFrame::MIN_DEPTH && depth <= DepthFrame::MAX_DEPTH);

                    // CLAMP
                    //valid = 1;
                    //depth -= ((depth - MAX_DEPTH) & -(depth > MAX_DEPTH));
                    //depth -= ((depth - MIN_DEPTH) & -(depth < MIN_DEPTH));

                    BYTE val = (BYTE)(255 - ((depth - DepthFrame::MIN_DEPTH) * 256) / (DepthFrame::MAX_DEPTH - DepthFrame::MIN_DEPTH));

                    *dest++ = val | -(1 - valid);
                    *dest++ = val & (-valid) & -(src->playerIndex == 0);
                    *dest++ = val & (-valid);
                    *dest++ = 0xFF;

                    ++src;
                }
            #endif
        }


        bool render() override
        {
            stream->getDepthFrame(frame, &frameNum);
            frameToImage();

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, DepthFrame::WIDTH, DepthFrame::HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DepthFrame::WIDTH, DepthFrame::HEIGHT,
                GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)textureBuffer);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, DepthFrame::WIDTH, DepthFrame::HEIGHT);
            RenderUtils::setTexture(0);

            return true;
        }

};

#endif
