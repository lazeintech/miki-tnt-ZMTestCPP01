#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>

using namespace std;

#define ELAPSED_TIME(MSG, ...) auto start = chrono::steady_clock::now(); __VA_ARGS__ ; auto end = chrono::steady_clock::now(); std::cout << "Elapsed time in milliseconds [" << MSG << "] : " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms\n";

class Solution
{
// #define VISUALIZE_RESULT
// #define ENABLE_DEBUG

// macroblock size
#define MB_1X1          (1)
#define MB_2X2          (2)
#define MB_4X4          (4)
#define MB_8X8          (8)
#define MB_SIZE         MB_1X1

// macroblock status
#define MB_IS_BLACK     (0)
#define MB_IS_MARKED    (1 << 0)
#define MB_IS_BORDER    (1 << 1)

    char   *buffer, *mb_map;
    size_t n, w, h;

public:
    Solution(char* buffer, size_t n, size_t w, size_t h)
    {
        this->buffer = buffer;
        this->n = n;
        this->w = w;
        this->h = h;

        // allocate memory for macroblock map
        mb_map = new char[(w * h) / (MB_SIZE * MB_SIZE)];
    }

    ~Solution()
    {
        delete[] mb_map;
    }

    void solve()
    {
        // width height in macroblock size
        auto const W_IN_MB = w / MB_SIZE;
        auto const H_IN_MB = h / MB_SIZE;

        // variables
#if   (MB_SIZE == MB_1X1)
        uint8_t *mb_line;
#elif (MB_SIZE == MB_2X2)
        uint16_t *mb_line;
#elif (MB_SIZE == MB_4X4)
        uint32_t *mb_line;
#elif (MB_SIZE == MB_8X8)
        uint64_t *mb_line;
#endif
        size_t i, mbid, line, row, col;
        size_t offset_addr, offset_id, object_cnt, mb_line_offset;
        size_t first_mbid, curr_mbid, prev_mbid;
        int    next_mbid;
        size_t top, btm, left, right, new_row, new_col;
        size_t drt, drt_it;
        DIRECTION prev_drt = DRT_TOP;

        std::ofstream result_file("result.txt");
        char object_info[50];

        auto mb_num      = (w * h) / (MB_SIZE * MB_SIZE);
        auto img_bytes   = w * h;
        auto is_empty    = true;
        auto is_inbound  = false;

        for (auto img = 0; img < n; ++img)
        {
            // process each image
#ifdef ENABLE_DEBUG
            printf("==== processing img #%d ====\n", img);
#endif
            offset_addr = (size_t)buffer + (img * img_bytes);
            offset_id   = img * img_bytes;
            object_cnt  = 0;

            // step 1: mark non-black macroblocks
            memset((void*)mb_map, MB_IS_BLACK, sizeof(mb_map[0]) * mb_num);
            for (mbid = 0; mbid < mb_num; ++mbid)
            {
                is_empty = true;
                mb_line_offset = (mbid / W_IN_MB) * MB_SIZE * w + (mbid % W_IN_MB) * MB_SIZE;
                for (line = 0; line < MB_SIZE; ++line)
                {
                    mb_line = (decltype(mb_line))(offset_addr + mb_line_offset + line*w);
                    if (*mb_line)
                    {
                        is_empty = false;
                        break;
                    }
                }
                if (!is_empty)
                {
                    mb_map[mbid] |= MB_IS_MARKED;
                }
            }

            // step 2: identify the object boundary
            for (row = 0; row < H_IN_MB; ++row)
            {
                is_inbound = false;
                for (col = 0; col < W_IN_MB; ++col)
                {
                    mbid = row * W_IN_MB + col;
                    /* seeking object */
                    if (is_inbound)
                    {
                        // non-marked -> outbound
                        if (!(mb_map[mbid] & MB_IS_MARKED))
                            is_inbound = false;

                        continue;
                    }

                    if (mb_map[mbid] & MB_IS_BORDER)
                    {
                        // found border of already detected object -> inbound
                        is_inbound = true;
                        continue;
                    }

                    // skip the black macroblock
                    if (!(mb_map[mbid] & MB_IS_MARKED))
                        continue;
                    // skip object's inbound macroblock
                    if (isBounded(mbid))
                        continue;

                    /* found object -> processing border */
                    prev_drt = DRT_TOP;
                    first_mbid = curr_mbid = prev_mbid = next_mbid = mbid;
                    top  = btm   = first_mbid / W_IN_MB;   // row
                    left = right = first_mbid % W_IN_MB;   // col
#ifdef ENABLE_DEBUG
                    printf("first_mbid mbid#%d %d-%d\n", first_mbid, left, top);
#endif

                    // process first macroblock of object's border
                    for (i = DRT_RIGHT; i <= DRT_LEFT; ++i)
                    {
                        next_mbid = getNeighborMBId(first_mbid, (DIRECTION)i);
                        if (next_mbid < 0) {
                            continue;
                        }
#ifdef ENABLE_DEBUG
                        printf("next_mbid %d %d-%d\n", next_mbid, next_mbid % W_IN_MB, next_mbid / W_IN_MB);
#endif
                        if (mb_map[next_mbid] & MB_IS_MARKED)
                        {
#ifdef ENABLE_DEBUG
                            printf("0 %s mbid#%d is %d\n", DirectionType[i], next_mbid, mb_map[next_mbid]);
                            buffer[offset_id + curr_mbid] = 255; // visualize boundary
#endif
                            prev_drt  = (DIRECTION)i;
                            curr_mbid = next_mbid;
                            break;
                        }
                    }
                    // process next macroblocks of object's border
                    while(curr_mbid != first_mbid)
                    {
                        // save edges
                        new_row = curr_mbid / W_IN_MB;
                        new_col = curr_mbid % W_IN_MB;
                        if (top > new_row)   top   = new_row;
                        if (btm < new_row)   btm   = new_row;
                        if (left > new_col)  left  = new_col;
                        if (right < new_col) right = new_col;

                        // adaptive direction border search
                        drt = getRevertDrt(prev_drt) + 1;
                        if (drt == DRT_MAX) drt = 0;
                        drt_it = drt;
                        do {
                            next_mbid = getNeighborMBId(curr_mbid, (DIRECTION)drt_it);
                            if (next_mbid < 0) {
                                ++drt_it;
                                if (drt_it == DRT_MAX) drt_it = 0;
                                continue;
                            }
                            if ((mb_map[next_mbid] & MB_IS_MARKED) && (next_mbid != prev_mbid || (mb_map[next_mbid] & MB_IS_BORDER)))
                            {
#ifdef ENABLE_DEBUG
                                printf("1 %s %s>%s mbid#%d %d-%d is %d\n",
                                        DirectionType[prev_drt], DirectionType[drt],  DirectionType[drt_it],
                                        next_mbid, new_col, new_row, mb_map[next_mbid]);
                                buffer[offset_id + curr_mbid] = 255; // visualize boundary
#endif
                                mb_map[curr_mbid] |= MB_IS_BORDER;

                                prev_drt  = (DIRECTION)drt_it;
                                prev_mbid = curr_mbid;
                                curr_mbid = next_mbid;
                                break;
                            }
                            ++drt_it;
                            if (drt_it == DRT_MAX) drt_it = 0;
                        }
                        while (drt_it != drt);
                    }
                    // process last macroblock of object's border
                    mb_map[curr_mbid] |= MB_IS_BORDER;
#ifdef ENABLE_DEBUG
                    buffer[offset_id + curr_mbid] = 255; // visualize boundary
#endif

                    // calculate coordinate
                    top         *= MB_SIZE;
                    btm         *= MB_SIZE;
                    left        *= MB_SIZE;
                    right       *= MB_SIZE;
                    btm         += (MB_SIZE - 1);
                    right       += (MB_SIZE - 1);
                    auto width  = (right - left);
                    auto height = (btm - top);

#ifdef ENABLE_DEBUG
                    printf("img#%d object#%d top left width height = %u %u %u %u\n",
                            img, ++object_cnt, top, left, width, height);
#endif
                    // output result to file
                    sprintf(object_info, "(%d, %d, %d, %d) ", top, left, width, height);
                    result_file << object_info;

#ifdef VISUALIZE_RESULT
                    // draw rectangle
                    for (i = left; i < right; ++i)
                    {
                        buffer[offset_id + top * w + i] = 255;
                        buffer[offset_id + btm * w + i] = 255;
                    }
                    for (i = 0; i < height; ++i)
                    {
                        buffer[offset_id + top * w + left  + i * w] = 255;
                        buffer[offset_id + top * w + right + i * w] = 255;
                    }
#endif
                }
            }
            result_file << "\n";

#ifdef ENABLE_DEBUG
            printf("bcnt: %d\n", bcnt);
            bcnt = 0;
#endif
        }

#ifdef VISUALIZE_RESULT
        // write output pictures with drawn rectangle to file
        std::ofstream outfile("output.bin");
        outfile.write(buffer, n*h*w);
        outfile.close();
#endif
        result_file.close();


    }

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

    int getNeighborMBId(size_t id, DIRECTION drt)
    {
        // TODO: validate wrapped border
        auto const W_IN_MB = w / MB_SIZE;
        switch (drt)
        {
        case DRT_TOPLEFT:
            return (id - W_IN_MB - 1);
        case DRT_TOP:
            return (id - W_IN_MB);
        case DRT_TOPRIGHT:
            return (id - W_IN_MB + 1);
        case DRT_RIGHT:
            return (id + 1);
        case DRT_BOTTOMRIGHT:
            return (id + W_IN_MB + 1);
        case DRT_BOTTOM:
            return (id + W_IN_MB);
        case DRT_BOTTOMLEFT:
            return (id + W_IN_MB - 1);
        case DRT_LEFT:
            return (id - 1);
        default:
            printf("ERROR: invalid drt %d\n", drt);
            return id;
        }
    }

    DIRECTION getRevertDrt(DIRECTION drt)
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
    }


#ifdef ENABLE_DEBUG
    int bcnt = 0;
#endif
    bool isBounded(size_t id)
    {
        auto const W_IN_MB = w / MB_SIZE;
        auto const H_IN_MB = h / MB_SIZE;
        int top = id;
        int btm = id;
        int lef = id;
        int rig = id;
        bool topBounded = false;
        bool btmBounded = false;
        bool lefBounded = false;
        bool rigBounded = false;

        while(top >= 0)
        {
#ifdef ENABLE_DEBUG
            ++bcnt;
#endif
            if (mb_map[top] & MB_IS_BORDER)
            {
                topBounded = true;
                break;
            }
            top -= W_IN_MB;
        }
        if (!topBounded) return false;

        auto first_col = id - ((id / W_IN_MB) * W_IN_MB);
        while(lef >= first_col)
        {
#ifdef ENABLE_DEBUG
            ++bcnt;
#endif
            if (mb_map[lef] & MB_IS_BORDER)
            {
                lefBounded = true;
                break;
            }
            lef -= 1;
        }
        if (!lefBounded) return false;

        auto max_mbid = W_IN_MB * H_IN_MB;
        while(btm <= max_mbid)
        {
#ifdef ENABLE_DEBUG
            ++bcnt;
#endif
            if (mb_map[btm] & MB_IS_BORDER)
            {
                btmBounded = true;
                break;
            }
            btm += W_IN_MB;
        }
        if (!btmBounded) return false;

        auto last_col = ((id / W_IN_MB) + 1) * W_IN_MB;
        while(rig <= last_col)
        {
#ifdef ENABLE_DEBUG
            ++bcnt;
#endif
            if (mb_map[rig] & MB_IS_BORDER)
            {
                rigBounded = true;
                break;
            }
            rig += 1;
        }
        if (!rigBounded) return false;

        return true;;
    };
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

    // solve
    Solution* sol = new Solution(buffer, n_images, width, height);
    ELAPSED_TIME("tada", sol->solve());
    delete sol;

    delete[] buffer;
    return 0;
}

