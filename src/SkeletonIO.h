#ifndef SKELETONIO_H
#define SKELETONIO_H

#pragma once

#include "globals.h"

// Handles the reading and writing of skeleton frames to files
class SkeletonIO
{
    private:

        // Bytes written as the first bytes of skeleton files,
        // to later identify if they are valid files.
        static const uint16_t MAGIC_NUMBER = 0xADDE;

        // The current file version
        static const int VERSION = 1;

        // The file header
        struct Header
        {
            uint16_t magicNumber;
            uint8_t version;
            uint8_t reserved;
            uint32_t numFrames;
        };

        FILE* file; // Current open file for reading/writing

        unsigned int currentFrame; // Current frame (reading) 
        unsigned int numFrames; // Total frame count (i.e. last written frame, while writing)

        bool writing; // True if writing, false if reading.

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

        // Close the current open file
        void close()
        {
            // If we were writing, rewrite the header
            if (writing) {
                fseek(file, 0, SEEK_SET);
                Header header = { 0 };
                header.magicNumber = MAGIC_NUMBER;
                header.version = VERSION;
                header.numFrames = numFrames;
                fwrite(&header, sizeof(header), 1, file);
                writing = false;
            }

            //  Close the file
            if (file != nullptr) {
                fclose(file);
                file = nullptr;
            }
        }


        // Returns true if a file is currently open
        bool isOpen() const
        {
            return file != nullptr;
        }


        //
        // FILE READING
        //

        // Opens the given file for reading skeleton frame(s). Returns true on success.
        bool openFileForReading(const char* filename)
        {
            file = fopen(filename, "rb");
            if (file == nullptr) return false;

            Header header;
            if (fread(&header, sizeof(header), 1, file) != 1) return false;

            // Is a skeleton frames file?
            if (header.magicNumber != MAGIC_NUMBER || header.version != VERSION) return false;

            numFrames = header.numFrames;
            currentFrame = 0;
            return true;
        }

        // Reads a single frame from the current open file. Returns true on success.
        bool readFrame(OUT SkeletonFrame& frame)
        {
            assert(file != nullptr && !writing);
            if (currentFrame >= numFrames) return false;
            ++currentFrame;
            return fread(&frame, sizeof(SkeletonFrame), 1, file) == 1;
        }

        // Reads the trajectory over time of the given set of joints.
        bool readTrajectory(const std::vector<NUI_SKELETON_POSITION_INDEX>& joints, OUT SkeletonTrajectory& trajectory)
        {
            assert(file != nullptr && !writing);
            long int originalPos = ftell(file);
            
            int n = int(joints.size());
            trajectory.numFrames = numFrames;
            trajectory.joints = joints;
            trajectory.points = new SkeletonTrajectory::Point[n * numFrames];
            
            SkeletonFrame* buffer = new SkeletonFrame();
            NUI_SKELETON_FRAME& f = buffer->frame;
            
            fseek(file, sizeof(Header), SEEK_SET);
            for (unsigned int k = 0; k < numFrames; ++k) {
                if (fread(buffer, sizeof(SkeletonFrame), 1, file) != 1) {
                    // ERROR
                }
                for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
                    if (f.SkeletonData[i].eTrackingState != NUI_SKELETON_NOT_TRACKED) {
                        for (int j = 0; j < n; ++j) {
                            SkeletonTrajectory::Point& p = trajectory.getPoint(j, k);
                            Vector4 v = f.SkeletonData[i].SkeletonPositions[joints[j]];
                            p.x = v.x;
                            p.y = v.y;
                            p.z = v.z;
                            p.state = f.SkeletonData[i].eSkeletonPositionTrackingState[joints[j]];
                        }
                        break;
                    }
                }
            }
            
            delete buffer;
            fseek(file, originalPos, SEEK_SET);
            return true;
        }
        
        
        // Starts reading the curent file again from the start.
        void reset()
        {
            assert(file != nullptr && !writing);
            fseek(file, sizeof(Header), SEEK_SET);
            currentFrame = 0;
        }

        // Gets the total number of frames in the file
        int getFrameCount() const
        {
            return numFrames;
        }


        //
        // FILE WRITING
        //

        // Opens the given file for writing skeleton frame(s). Returns true on success.
        bool openFileForWriting(const char* filename)
        {
            qDebug() << "Opening " << QString(filename);
            file = fopen(filename, "wb");
            if (file == nullptr) return false;

            Header header = { 0 };
            if (fwrite(&header, sizeof(header), 1, file) != 1) {
                fclose(file);
                file = nullptr;
                return false;
            }
            writing = true;
            return true;
        }

        // Writes a single skeleton frame to the current open file.
        void writeFrame(const SkeletonFrame& frame)
        {
            assert(file != nullptr && writing);
            fwrite(&frame, sizeof(SkeletonFrame), 1, file);
            ++numFrames;
        }

		bool hasFinished() const {
			return currentFrame >=numFrames;
		}
};

#endif
