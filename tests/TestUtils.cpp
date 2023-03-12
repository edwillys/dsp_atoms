#include "TestUtils.h"
#include <sndfile.h>
#include <iostream>
#include <fstream>

//=============================================================
// Helper functions
//=============================================================

std::vector<std::string> split(const std::string &s, const std::string &delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void write_wav(const std::string &outwav, const std::vector<float32_t> &content,
               cint32_t fs, cint32_t bps, cint32_t nch)
{
    // write that same content to another file
    SF_INFO info;
    info.channels = nch;
    info.samplerate = fs;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE *const sndfile = sf_open(outwav.c_str(), SFM_WRITE, &info);

    if (sndfile == NULL)
    {
        std::cout << "Something went wrong in out open" << std::endl;
    }
    else
    {
        sf_count_t count = sf_write_float(sndfile, content.data(), content.size());
        sf_write_sync(sndfile);
        if (count != content.size())
        {
            std::cout << "Something went wrong in write" << std::endl;
        }
        sf_close(sndfile);
    }
}

std::vector<float32_t> read_wav(const std::string &path_wav, cint32_t ch)
{
    // write that same content to another file
    SF_INFO info;
    SNDFILE *const sndfile = sf_open(path_wav.c_str(), SFM_READ, &info);
    std::vector<float32_t> wav;

    if (sndfile == NULL)
    {
        std::cout << "Something went wrong in out open" << std::endl;
    }
    else
    {
        if (ch < info.channels)
        {
            wav.resize(info.frames);
            // Read data
            sf_seek(sndfile, ch * info.frames, SEEK_SET);
            sf_read_float(sndfile, &wav[0], info.frames);
            sf_close(sndfile);
        }
    }

    return wav;
}

bool compare_wav(const std::string &wavL, const std::string &wavR, float32_t eps)
{
    SF_INFO info[2];
    SNDFILE *const sndfile[2] = {
        sf_open(wavL.c_str(), SFM_READ, &info[0]),
        sf_open(wavR.c_str(), SFM_READ, &info[1]),
    };

    bool retval = true;
    if (
        !sndfile[0] ||
        !sndfile[1] ||
        info[0].channels != info[1].channels ||
        info[0].samplerate != info[1].samplerate ||
        info[0].frames != info[1].frames)
    {
        std::cerr << "WAV files have different info " << std::endl;
        sf_close(sndfile[0]);
        sf_close(sndfile[1]);
        retval = false;
    }
    else
    {
        // Read data
        std::vector<float32_t> buf_ref[2];
        buf_ref[0].resize(info[0].frames);
        buf_ref[1].resize(info[1].frames);

        sf_seek(sndfile[0], 0ul, SEEK_SET);
        sf_seek(sndfile[1], 0ul, SEEK_SET);

        sf_read_float(sndfile[0], &buf_ref[0][0], info[0].frames);
        sf_read_float(sndfile[1], &buf_ref[1][0], info[1].frames);

        for (auto j = 0; j < info[0].frames; j++)
        {
            if (fabsf(buf_ref[0][j] - buf_ref[1][j]) > eps)
            {
                retval = false;
                break;
            }
        }

        sf_close(sndfile[0]);
        sf_close(sndfile[1]);
    }

    return retval;
}

bool compare_csv(const std::string &left, const std::string &right, float32_t eps)
{
    bool retval = true;

    std::ifstream ifs_left(left);
    std::ifstream ifs_right(right);
    std::string line_left;
    std::string line_right;

    if (ifs_left.is_open() && ifs_right.is_open())
    {
        while (std::getline(ifs_left, line_left) && std::getline(ifs_right, line_right))
        {
            auto line_split_left = split(line_left, ",");
            auto line_split_right = split(line_right, ",");

            if (line_split_left.size() != line_split_right.size())
            {
                retval = false;
                break;
            }
            else
            {
                for (auto i = 0; i < line_split_left.size(); i++)
                {
                    auto float_left = std::atof(line_split_left[i].c_str());
                    auto float_right = std::atof(line_split_right[i].c_str());
                    if (fabsf(float_left - float_right) > eps)
                    {
                        retval = false;
                        break;
                    }
                }
            }
        }

        ifs_left.close();
        ifs_right.close();
    }
    else
    {
        retval = false;
    }

    return retval;
}
