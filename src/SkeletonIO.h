#ifndef SKELETONIO_H
#define SKELETONIO_H

#pragma once

#include "globals.h"

// Handles the reading and writing of skeleton frames to files
class SkeletonIO
{
    private:

        static const uint16_t MAGIC_NUMBER = 0xADDE;

        struct Header
        {
            uint16_t magicNumber;
            uint8_t version;
            uint8_t reserved;
            uint32_t numFrames;
        };

        FILE* file;

        unsigned int currentFrame;
        unsigned int numFrames;

        bool writing;

    public:

        SkeletonIO() :
            file(nullptr),
            currentFrame(),
            numFrames(),
            writing(false)
        {

        }

        ~SkeletonIO()
        {
            close();
        }

        void close()
        {
            if (writing) { // Write header
                fseek(file, 0, SEEK_SET);
                Header header = { 0 };
                header.magicNumber = MAGIC_NUMBER;
                header.version = 1;
                header.numFrames = numFrames;
                fwrite(&header, sizeof(header), 1, file);
                writing = false;
            }
            if (file != nullptr) {
                fclose(file);
                file = nullptr;
            }
        }

        bool isOpen() const
        {
            return file != nullptr;
        }


        // FILE READING

        bool openFileForReading(const char* filename)
        {
            file = fopen(filename, "rb");
            if (file == nullptr) return false;

            Header header;
            if (fread(&header, sizeof(header), 1, file) != 1) return false;

            if (header.magicNumber != MAGIC_NUMBER || header.version != 1) return false;
            numFrames = header.numFrames;
            currentFrame = 0;
            return true;
        }

        bool readFrame(OUT SkeletonFrame& frame)
        {
            assert(file != nullptr && !writing);
            if (currentFrame >= numFrames) return false;
            ++currentFrame;
            return fread(&frame, sizeof(SkeletonFrame), 1, file) == 1;
        }

        void reset()
        {
            assert(file != nullptr && !writing);
            fseek(file, sizeof(Header), SEEK_SET);
            currentFrame = 0;
        }


        // FILE WRITING

        bool openFileForWriting(const char* filename)
        {
            file = fopen(filename, "wb");
            if (file == nullptr) return false;

            Header header = { 0 };
            fwrite(&header, sizeof(header), 1, file);
            writing = true;
            return true;
        }

        void writeFrame(const SkeletonFrame& frame)
        {
            assert(file != nullptr && writing);
            fwrite(&frame, sizeof(SkeletonFrame), 1, file);
            ++numFrames;
        }

};

#endif
