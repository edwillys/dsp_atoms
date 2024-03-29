#include "TestUtils.h"
#include <sndfile.h>
#include <iostream>
#include <fstream>
#include <sstream>

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

std::vector<std::string> split(const std::string &s, const char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

std::vector<std::vector<float32_t>> deinterleave(const std::vector<float32_t> &in, cint32_t nch)
{
    std::vector<std::vector<float32_t>> out(nch);
    cint32_t len = (cint32_t)in.size() / nch;

    for (auto ch = 0; ch < nch; ch++)
        out[ch].resize(len);

    for (auto ch = 0; ch < nch; ch++)
    {
        for (auto i = 0; i < len; i++)
        {
            out[ch][i] = in[i * nch + ch];
        }
    }

    return out;
}

std::vector<float32_t> interleave(const std::vector<std::vector<float32_t>> &in)
{
    auto nch = in.size();
    auto len = in[0].size();
    std::vector<float32_t> out(nch * len);

    for (auto ch = 0; ch < nch; ch++)
    {
        for (auto i = 0; i < len; i++)
        {
            out[i * nch + ch] = in[ch][i];
        }
    }

    return out;
}

void write_wav(const std::string &outwav, const std::vector<float32_t> &content,
               cint32_t fs, cint32_t bps, cint32_t nch)
{
    // write that same content to another file
    SF_INFO info;
    info.channels = nch;
    info.samplerate = fs;
    info.frames = content.size();
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

void write_wav(const std::string &outwav, const std::vector<std::vector<float32_t>> &content,
               cint32_t fs, cint32_t bps)
{
    std::vector<float32_t> content_interleaved = interleave(content);
    write_wav(outwav, content_interleaved, fs, bps, (cint32_t)content.size());
}

std::vector<std::vector<float32_t>> read_wav(const std::string &path_wav)
{
    // write that same content to another file
    SF_INFO info;
    SNDFILE *const sndfile = sf_open(path_wav.c_str(), SFM_READ, &info);
    std::vector<std::vector<float32_t>> wav;

    if (sndfile == NULL)
    {
        std::cout << "Something went wrong in out open" << std::endl;
    }
    else
    {
        std::vector<float32_t> wavInterleaved(info.channels * info.frames);
        // Read data
        sf_read_float(sndfile, &wavInterleaved[0], info.channels * info.frames);
        sf_close(sndfile);
        wav = deinterleave(wavInterleaved, info.channels);
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

        float32_t max_diff = 0.F; // for debugging 
        for (auto j = 0; j < info[0].frames; j++)
        {
            auto diff = fabs(buf_ref[0][j] - buf_ref[1][j]);
            max_diff = fmax(max_diff, diff);
            if (diff > eps)
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
            auto line_split_left = split(line_left, ',');
            auto line_split_right = split(line_right, ',');

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
                    if (fabs(float_left - float_right) > eps)
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
