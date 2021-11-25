#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <chrono>
#include <unistd.h>

using namespace std;

#define ELAPSED_TIME(MSG, ...) auto start = chrono::steady_clock::now(); __VA_ARGS__ ; auto end = chrono::steady_clock::now(); std::cout << "Elapsed time in milliseconds [" << MSG << "] : " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms\n";

class Solution
{
#define MB_SIZE 1

private:
    enum DIRECTION
    {
        DRT_TOPLEFT = 0,
        DRT_TOP,
        DRT_TOPRIGHT,
        DRT_RIGHT,
        DRT_BOTTOMRIGHT,
        DRT_BOTTOM,
        DRT_BOTTOMLEFT,
        DRT_LEFT,
        DRT_MAX
    };
    char *DirectionType[DRT_MAX] =
    {
        (char*)"DRT_TOPLEFT     ",
        (char*)"DRT_TOP         ",
        (char*)"DRT_TOPRIGHT    ",
        (char*)"DRT_RIGHT       ",
        (char*)"DRT_BOTTOMRIGHT ",
        (char*)"DRT_BOTTOM      ",
        (char*)"DRT_BOTTOMLEFT  ",
        (char*)"DRT_LEFT        "
    };

    class MBNode
    {
    public:
        MBNode() {}
        
        MBNode(size_t mb, size_t w)
        {
            this->mb = mb;
            this->w  = w;
            this->x = (mb % (w/MB_SIZE)) * MB_SIZE;
            this->y = (mb / (w/MB_SIZE)) * MB_SIZE;
        }

        auto getX()
        {
            return (mb % (w/MB_SIZE)) * MB_SIZE;
        }
        auto getY()
        {
            return (mb / (w/MB_SIZE)) * MB_SIZE;
        }

        // todo isEqual xx yy
        size_t mb;
        bool isMarked;
        bool isBorder;
        bool isRaised;
        bool isFellen;
        size_t x;
        size_t y;
        size_t w;
    };

    char* buffer;
    MBNode* dist_map;
    size_t n;
    size_t w;
    size_t h;

public:

    Solution(char* buffer, size_t n, size_t w, size_t h)
    {
        this->buffer = buffer;
        this->n = n;
        this->w = w;
        this->h = h;

        dist_map = new MBNode[(w * h) / (MB_SIZE * MB_SIZE)];
    }

    ~Solution()
    {
        delete[] dist_map;
    }

    int getNeighborMBId(size_t mb, DIRECTION drt)
    {
        auto const W_IN_MB = w / MB_SIZE;
        switch (drt)
        {
        case DRT_TOPLEFT:
            return (mb - W_IN_MB - 1);
        case DRT_TOP:
            return (mb - W_IN_MB);
        case DRT_TOPRIGHT:
            return (mb - W_IN_MB + 1);
        case DRT_RIGHT:
            return (mb + 1);
        case DRT_BOTTOMRIGHT:
            return (mb + W_IN_MB + 1);
        case DRT_BOTTOM:
            return (mb + W_IN_MB);
        case DRT_BOTTOMLEFT:
            return (mb + W_IN_MB - 1);
        case DRT_LEFT:
            return (mb - 1);
        default:
            printf("ERROR: invalid drt %d\n", drt);
            return mb;
        }
        // TODO out of range top btn left right
    }

    auto getRevertDrt(DIRECTION drt)
    {
        switch (drt)
        {
        case DRT_TOPLEFT:
            return DRT_BOTTOMRIGHT;
            break;
        case DRT_TOP:
            return DRT_BOTTOM;
            break;
        case DRT_TOPRIGHT:
            return DRT_BOTTOMLEFT;
            break;
        case DRT_RIGHT:
            return DRT_LEFT;
            break;
        case DRT_BOTTOMRIGHT:
            return DRT_TOPLEFT;
            break;
        case DRT_BOTTOM:
            return DRT_TOP;
            break;
        case DRT_BOTTOMLEFT:
            return DRT_TOPRIGHT;
            break;
        case DRT_LEFT:
            return DRT_RIGHT;
            break;
        default:
            return DRT_MAX;
            break;
        }
        // TODO out of range top btn left right
    }

    auto isBounded(size_t mb)
    {
        auto const W_IN_MB = w / MB_SIZE;
        auto const H_IN_MB = h / MB_SIZE;
        int top = mb;
        int btm = mb;
        int lef = mb;
        int rig = mb;
        bool topBounded = false;
        bool btmBounded = false;
        bool lefBounded = false;
        bool rigBounded = false;

        while(top >= 0)
        {
            if (dist_map[top].isBorder)
            {
                topBounded = true;
                break;
            }
            top -= W_IN_MB;
        }
        while(btm <= (W_IN_MB * H_IN_MB))
        {
            if (dist_map[btm].isBorder)
            {
                btmBounded = true;
                break;
            }
            btm += W_IN_MB;
        }
        while(lef >= (mb - ((mb/W_IN_MB) * W_IN_MB)))
        {
            if (dist_map[lef].isBorder)
            {
                lefBounded = true;
                break;
            }
            lef -= 1;
        }
        while(rig <= ((mb / W_IN_MB) + 1) * W_IN_MB)
        {
            if (dist_map[rig].isBorder)
            {
                rigBounded = true;
                break;
            }
            rig += 1;
        }
        return (topBounded && btmBounded && lefBounded && rigBounded);
    };
    
    void solve()
    {
        auto const W_IN_MB = w / MB_SIZE;
        auto const H_IN_MB = h / MB_SIZE;

#if(MB_SIZE == 1)
        uint8_t *mb_line;
#elif(MB_SIZE == 2)
        uint16_t *mb_line;
#elif(MB_SIZE == 4)
        uint32_t *mb_line;
#elif(MB_SIZE == 8)
        uint64_t *mb_line;
#else
#error Invalid MB_SIZE (must be 1, 2, 4 or 8)
#endif
        size_t i, mb, line, x, y;
        auto mb_num = (w * h) / (MB_SIZE * MB_SIZE);
        auto img_bytes = w * h;
        auto empty = true;

        for (auto img = 0; img < n; ++img)
        {
            /** process each image **/
            printf("==== processing img #%d ====\n", img);
            auto offset_addr = (size_t)buffer + (img * img_bytes);

            // step 1: mark non-black macroblocks
            memset((void*)dist_map, 0, sizeof(MBNode) * W_IN_MB * H_IN_MB);
            for (mb = 0; mb < mb_num; ++mb)
            {
                empty = true;
                for (line = 0; line < MB_SIZE; ++line)
                {
                    mb_line = (decltype(mb_line))(offset_addr + ( (mb/W_IN_MB)*MB_SIZE*w + (mb%W_IN_MB)*MB_SIZE ) + line*w);
                    if (*mb_line)
                    {
                        empty = false;
                        break;
                    }
                }
                if (!empty)
                {
                    x = (mb % (w/MB_SIZE)) * MB_SIZE;
                    y = (mb / (w/MB_SIZE)) * MB_SIZE;

                    dist_map[mb].isMarked = true;
                    dist_map[mb].mb = mb;
                }
            }

            // step 1.1: mark raise and fall
            if (1)
            {
                // identify raise and fall
                for (auto row = 0; row < H_IN_MB; ++row)
                {
                    auto raised = false;
                    for (auto col = 0; col < W_IN_MB; ++col)
                    {
                        if (raised)
                        {
                            if (dist_map[row*W_IN_MB+ col].isMarked)
                            {
                                raised = true;
                            }
                            else
                            {
                                dist_map[row*W_IN_MB+ col - 1].isFellen = true;
                                // dist_map[row*W_IN_MB+ col - 1].isBorder = true;
                                raised = false;
                            }
                            continue;
                        }

                        if (dist_map[row*W_IN_MB+ col].isMarked)
                        {
                            dist_map[row*W_IN_MB+ col].isRaised = true;
                            // dist_map[row*W_IN_MB+ col].isBorder = true;
                            raised = true;
                        }
                    }
                }
            }

            // step 2: identify the boundary, save and blackout 

            auto shape_cnt = 0;
            for (auto row = 0; row < H_IN_MB; ++row)
            {
                auto border_raised = false;
                for (auto col = 0; col < W_IN_MB; ++col)
                {
                    if (border_raised)
                    {
                        // if (dist_map[row*W_IN_MB+ col].isFellen)
                        if (!dist_map[row*W_IN_MB+ col].isMarked)
                        {
                            border_raised = false;
                        }
                        if (dist_map[row*W_IN_MB+ col].isBorder)
                        {
                            continue;
                        }
                        continue;
                    }

                    if (dist_map[row*W_IN_MB+ col].isBorder)
                    {
                        border_raised = true;
                        continue;
                    }

                    if (dist_map[row*W_IN_MB+ col].isMarked)
                    {
                        if (isBounded(row*W_IN_MB+ col))
                        {
                            continue;
                        }
                        //do things here
                        // identify
                        auto start_mb = &dist_map[row*W_IN_MB+ col];
                        auto curr_mb = start_mb;
                        auto prev_mb = start_mb;
                        // printf("mb#%d %d-%d\n", start_mb->mb, start_mb->mb%w,  start_mb->mb/w);

                        auto prev_drt = DRT_TOP;
                        size_t top ,btm , left, right;
                        top = btm = curr_mb->mb/w;      // row
                        left = right = curr_mb->mb%w;   // col
                        // pre process
                        for (i = DRT_RIGHT; i <= DRT_LEFT; ++i)
                        {
                            auto nextMBId = getNeighborMBId(start_mb->mb, (DIRECTION)i);
                            if (nextMBId < 0) {
                                continue;
                                // top line TODO: handle left and right line?
                                // if (!dist_map[getNeighborMBId(start_mb->mb, DRT_RIGHT)
                                // nextMBId = start_mb->mb + 1; // TODO move into getNeighbor
                            }
                            // printf("nextMBId %d %d-%d\n", nextMBId, nextMBId % w, nextMBId / w);
                            if (dist_map[nextMBId].isMarked)
                            {
                                // printf("0 %s mb#%d is %d\n", DirectionType[i], nextMBId, dist_map[nextMBId].isMarked);

                                // buffer[img*w*h + curr_mb->mb] = 255; // for test

                                prev_drt = (DIRECTION)i;
                                curr_mb = &dist_map[nextMBId];
                                break;
                            }
                        }
                        // process
                        while(curr_mb->mb != start_mb->mb)
                        {
                            // save
                            if (top > curr_mb->mb/w)
                            {
                                top = curr_mb->mb/w;
                            }
                            if (btm < curr_mb->mb/w)
                            {
                                btm = curr_mb->mb/w;
                            }
                            if (left > curr_mb->mb%w)
                            {
                                left = curr_mb->mb%w;
                            }
                            if (right < curr_mb->mb%w)
                            {
                                right = curr_mb->mb%w;
                            }

                            int drt = getRevertDrt(prev_drt) + 1;
                            if (drt == DRT_MAX) drt = 0;
                            auto drt_it = drt;
                            do {
                                auto nextMBId = getNeighborMBId(curr_mb->mb, (DIRECTION)drt_it);
                                if (nextMBId < 0) {
                                    ++drt_it;
                                    if (drt_it == DRT_MAX) drt_it = 0;
                                    continue;
                                    // top line TODO: handle left and right line?
                                    // if (!dist_map[getNeighborMBId(start_mb->mb, DRT_RIGHT)
                                    // nextMBId = start_mb->mb + 1; // TODO move into getNeighbor
                                }
                                // if (dist_map[nextMBId].isMarked && dist_map[nextMBId].mb != prev_mb->mb) //TODO impl equal MB
                                if (dist_map[nextMBId].isMarked && (dist_map[nextMBId].mb != prev_mb->mb || dist_map[nextMBId].isBorder)) //TODO impl equal MB
                                // TODO turn around in single line
                                {
                                    // printf("1 %s %s>%s mb#%d %d-%d is %d\n", DirectionType[prev_drt], DirectionType[drt],  DirectionType[drt_it], nextMBId, curr_mb->mb%w,  curr_mb->mb/w, dist_map[nextMBId].isMarked);
                                    
                                    // buffer[img*w*h + curr_mb->mb] = 255; // for test

                                    curr_mb->isBorder = true;
                                    
                                    prev_drt = (DIRECTION)drt_it;
                                    prev_mb = curr_mb;
                                    curr_mb = &dist_map[nextMBId];
                                    break;
                                }
                                ++drt_it;
                                if (drt_it == DRT_MAX) drt_it = 0;
                            }
                            while (drt_it != drt);
                        }
                        // post process
                        {
                            // buffer[img*w*h + curr_mb->mb] = 255; // for test
                            curr_mb->isBorder = true;
                        }
                        // fill with white
                        while(curr_mb->mb != start_mb->mb)
                        {

                        }

                        // draw rectangle
                        printf("img#%d no.%d top left w h = %d %d %d %d\n", img,  ++shape_cnt, top, left, right - left, btm - top);
                        for (i = left; i < right; ++i)
                        {
                            buffer[img*w*h + top*W_IN_MB + i] = 255;
                            buffer[img*w*h + btm*W_IN_MB + i] = 255;
                        }
                        for (i = 0; i < btm - top; ++i)
                        {
                            buffer[img*w*h + top*W_IN_MB + left  + i*W_IN_MB] = 255;
                            buffer[img*w*h + top*W_IN_MB + right + i*W_IN_MB] = 255;
                        }

                    }
                }

            }
            std::ofstream outfile("output.bin");
            outfile.write(buffer, n*h*w);
            outfile.close();

            // break;
        }
    }

};

int main() {
    const size_t n_images = 10;
    const size_t width = 800, height = 600;

    //open file
    std::ifstream infile("data.bin");

    // get length of file
    infile.seekg(0, std::ios::end);
    std::streamsize length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    auto* buffer = new char[n_images * height * width];

    // read whole file
    infile.read(buffer, length);

    Solution* sol = new Solution(buffer, n_images, width, height);
    ELAPSED_TIME("tada", sol->solve());
    delete sol;

    delete[] buffer;
    return 0;
}

