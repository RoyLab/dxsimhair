#include <exception>
#include <string>
#include "CacheHair.h"


namespace WR
{
    CacheHair::~CacheHair()
    {
        SAFE_DELETE_ARRAY(position);
    }

    void CacheHair::BinaryHelper::init(size_t &nf, size_t &np)
    {
        char bytes[4];
        file.read(bytes, 4);
        nf = *reinterpret_cast<int*>(bytes);

        file.read(bytes, 4);
        np = *reinterpret_cast<int*>(bytes);
    }
    void CacheHair::BinaryHelper::readFrame(float* pos, size_t np)
    {
        char* bytes = reinterpret_cast<char*>(pos);
        file.read(bytes, sizeof(float)*np*3);
    }

    bool CacheHair::BinaryHelper::hasNextFrame(size_t &id)
    {
        char bytes[4];
        file.read(bytes, 4);

        if (file.eof())
            return false;

        id = *reinterpret_cast<int*>(bytes);
        return true;
    }

    void CacheHair::AsciiHelper::init(size_t &nf, size_t &np)
    {
        file >> nf;
        file >> np;
    }
    void CacheHair::AsciiHelper::readFrame(float* pos, size_t np)
    {
        std::string line;
        for (size_t i = 0; i < np; i++)
        {
            for (size_t j = 0; j < 3; j++)
                file >> pos[3 * i + j];
        }
    }
    bool CacheHair::AsciiHelper::hasNextFrame(size_t &id)
    {
        std::string line;
        std::getline(file, line);

        do
        {
            std::getline(file, line);
        } while (!file.eof() && line.substr(0, 6) != "Frame ");

        if (file.eof()) return false;

        std::string numberStr = line.substr(6, line.size());
        id = std::atoi(numberStr.c_str());
        return true;
    }

    bool CacheHair::loadFile(const char* fileName, bool binary)
    {
        if (binary)
        {
            file = std::ifstream(fileName, std::ios::binary);
            helper = new BinaryHelper(file);
        }
        else
        {
            file = std::ifstream(fileName);
            helper = new AsciiHelper(file);
        }
        if (!file.is_open()) throw std::exception("file not found!");

        helper->init(m_nFrame, m_nParticle);

        position = new float[3 * m_nParticle];
        firstFrame = file.tellg();
        bNextFrame = true;
        return true;
    }

    void CacheHair::rewind()
    {
        file.clear();
        file.seekg(firstFrame);
    }

    size_t CacheHair::getFrameNumber() const
    {
        return get_nFrame();
    }

    size_t CacheHair::n_strands() const
    {
        return get_nParticle() / N_PARTICLES_PER_STRAND;
    }

    const float* CacheHair::get_visible_particle_position(size_t i, size_t j) const
    {
        return position + (i*N_PARTICLES_PER_STRAND+j)*3;
    }

    void CacheHair::onFrame(Mat3 world, float fTime, float fTimeElapsed, void*)
    {
        if (bNextFrame)
        {
            if (hasNextFrame())
                readFrame();
            else
            {
                rewind();
                hasNextFrame();
                readFrame();
            }
        }
    }

    void CacheHair::readFrame()
    {
        helper->readFrame(position, get_nParticle());
    }

    bool CacheHair::hasNextFrame()
    {
        return helper->hasNextFrame(m_curFrame);
    }
}