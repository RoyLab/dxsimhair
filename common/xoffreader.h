#pragma once
#include <cassert>
#include <string>
#include <fstream>
#include <cctype>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

#include "cy\cyPoint.h"

namespace XR
{
    inline bool isempty(const char *s)
    {
        while (*s) {
            if (!std::isspace(*s)) return false;
            s++;
        }
        return true;
    }

    typedef std::vector<cyPoint3f>  VertexStorage;

    class FaceStorage
    {
    public:
        virtual int number_of_faces() const = 0;
        virtual void get_face_index(std::vector<int>&, int) const = 0;
        virtual int get_face_degree(int) const = 0;
        virtual void add_face(const std::vector<int>&) = 0;
    };

    class TriFaceStorage:
        public FaceStorage
    {
    public:
        int number_of_faces() const { return data_.size(); }

        void get_face_index(std::vector<int>& o, int id) const
        {
            int n = get_face_degree(id);
            o.resize(n);
            for (int i = 0; i < n; ++i)
            {
                o[i] = data_[id][i];
            }
        }

        int get_face_degree(int id) const { return 3; }

        void add_face(const std::vector<int>& face)
        {
            assert(face.size() == 3);
            data_.emplace_back();
            for (int i = 0; i < 3; ++i)
            {
                data_.back[i] = face[i];
            }
        }

    private:
        std::vector<int[3]> data_;
    };

    class PolyFaceStorage :
        public FaceStorage
    {
    public:
        int number_of_faces() const { return data_.size(); }

        void get_face_index(std::vector<int>& o, int id) const
        {
            int n = get_face_degree(id);
            o.resize(n);
            for (int i = 0; i < n; ++i)
            {
                o[i] = data_[indices_[id] + i];
            }
        }

        int get_face_degree(int id) const
        {
            return indices_[id + 1] - indices_[id];
        }

        void add_face(const std::vector<int>& face)
        {
            assert(face.size());
            if (indices_.empty())
            {
                indices_.push_back(0);
            }

            data_.insert(data_.end(), face.begin(), face.end());
            indices_.push_back(data_.size());
        }

    private:
        std::vector<int> indices_;
        std::vector<int> data_;
    };


    class OffMesh
    {
    public:
        std::string     filename;
        std::ifstream   file;
        size_t          nv, nf, ne;
        bool            is_tri_mesh;

        std::unique_ptr<VertexStorage>  vertices = nullptr;
        std::unique_ptr<FaceStorage>  faces = nullptr;

    private:
        friend class OffMeshLoader;
        enum STAGE { HEAD, VERTEX, FACE, END };
        STAGE state = HEAD;
    };

    class OffMeshLoader
    {
    public:
        OffMeshLoader() {}
        ~OffMeshLoader() {}

        OffMesh* load(const char* fname, bool is_trimesh) const
        {
            std::string posfix(".off");
            int n = std::strlen(fname);
            if (std::strcmp(fname + n - posfix.size(), posfix.c_str()) != 0)
            {
                throw std::exception("Wrong format");
            }

            OffMesh* ret = new OffMesh;
            ret->file = std::ifstream(fname);
            if (!ret->file.is_open())
            {
                const int sz = 100;
                char ch[sz];
                std::snprintf(ch, sz, "Cannnot open file %s", fname);
                throw std::exception(ch);
            }

            ret->filename = fname;
            ret->is_tri_mesh = is_trimesh;

            std::string header;
            ret->file >> header;
            if (header != "OFF")
            {
                throw std::exception("Wrong syntax");
            }

            ret->file >> ret->nv >> ret->nf >> ret->ne;

            if (ret->nv == 0 && ret->nf == 0)
            {
                throw std::exception("Empty off file");
            }

            ret->state = OffMesh::VERTEX;
            return ret;
        }

        void load_vertices(OffMesh& mesh)
        {
            if (mesh.vertices || mesh.state != OffMesh::VERTEX)
            {
                throw std::exception("Cannot load vertices");
            }

            mesh.vertices.reset(new VertexStorage);
            char line[256];
            for (int i = 0; i < mesh.nv; ++i)
            {
                do
                {
                    mesh.file.getline(line, 256);
                } while (isempty(line));

                int degree, id, last_id, id_head;
                std::istringstream sline(line);

                mesh.vertices->emplace_back();
                for (int j = 0; j < 3; ++j)
                {
                    sline >> mesh.vertices->back()[j];
                }
            }

            mesh.state = OffMesh::FACE;
        }

        void load_faces(OffMesh& mesh)
        {
            if (mesh.faces || mesh.state != OffMesh::FACE)
            {
                throw std::exception("Cannot load vertices");
            }

            if (mesh.is_tri_mesh)
                mesh.faces.reset(new TriFaceStorage);
            else
                mesh.faces.reset(new PolyFaceStorage);

            char line[256];
            std::vector<int> face;
            for (int i = 0; i < mesh.nf; ++i)
            {
                do
                {
                    mesh.file.getline(line, 256);
                } while (isempty(line));

                int degree;
                std::istringstream sline(line);
                sline >> degree;
                face.resize(degree);

                for (int j = 0; j < degree; ++j)
                    sline >> face[j];

                mesh.faces->add_face(face);
            }

            mesh.state = OffMesh::END;
        }
    };



}
