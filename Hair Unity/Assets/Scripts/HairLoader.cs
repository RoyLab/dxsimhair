using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

/// <summary>
/// HairLoader:
/// A helper class for loading the hair model from .hair extension file
/// </summary>
public class HairLoader {
    public static List<GameObject> LoadStaticHair(string name, int limit = 60000) {
        List<GameObject> ret = new List<GameObject>();

        var asset = Resources.Load(name) as TextAsset;
        using (var reader = new BinaryReader(new MemoryStream(asset.bytes)))
        {
            var pointsCount = reader.ReadInt32();
            var points = new Vector3[pointsCount];
            for (int i = 0; i < pointsCount; ++i)
                points[i] = new Vector3(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
            var linesCount = reader.ReadInt32();
            var linesPointCount = new int[linesCount];
            for (int i = 0; i < linesCount; ++i)
                linesPointCount[i] = reader.ReadInt32();

            for (int i = 0, pointCur = 0; i < linesCount;) {
                Color currentColor = Random.ColorHSV(0.0f, 1.0f, 0.6f, 0.6f, 0.7f, 0.7f);
                int iEnd = i;
                int totalPoints = 0;
                while (iEnd < linesCount && totalPoints + linesPointCount[iEnd] < limit)
                    totalPoints += linesPointCount[iEnd++];

                var hairObject = CreateHairObject(name);

                var hairObjectIndexer = hairObject.GetComponent<HairIndexer>();
                hairObjectIndexer.hairBeginIndex = i;
                hairObjectIndexer.particleBeginIndex = pointCur;
                hairObjectIndexer.hairEndIndex = iEnd;
                hairObjectIndexer.particleEndIndex = pointCur + totalPoints;

                var hairObjectMesh = hairObject.GetComponent<MeshFilter>().mesh;

                var vertices = new Vector3[totalPoints];
                var colors = new Color[totalPoints];
                var indices = new int[2 * (totalPoints - (iEnd - i))];
                for (int p = pointCur, ic = 0, lc = i, lcPointCur = 0; p < pointCur + totalPoints; ++p) {
                    //p indicates the current scanning position in the "points" array, which means p - pointCur is the current position in the "vertices" array
                    //vc indicates the current vertex position, vc = p - pointCur
                    //ic indicates the current poistion in the "indices" array
                    //lc indices the current scanning position in the lines
                    //lcPointCur indices the current point position in lines[lc]
                    int vc = p - pointCur;
                    vertices[vc] = points[p];
                    colors[vc] = currentColor;

                    //if it is not the last point in the line
                    if (lcPointCur < linesPointCount[lc] - 1) {
                        indices[ic] = vc;
                        indices[ic + 1] = vc + 1;
                        ic += 2;
                        ++lcPointCur;
                    } 
                    else {
                        ++lc;
                        lcPointCur = 0;
                    }
                }

                hairObjectMesh.vertices = vertices;
                hairObjectMesh.colors = colors;
                hairObjectMesh.SetIndices(indices, MeshTopology.Lines, 0);
                hairObjectMesh.RecalculateBounds();

                ret.Add(hairObject); 

                i = iEnd;
                pointCur += totalPoints;
            }
        }
        return ret;
    }

    public static List<GameObject> Load(Vector3[] points, int[] pointsPerStrand, int limit = 60000)
    {
        List<GameObject> ret = new List<GameObject>();
        for (int i = 0, pointCur = 0; i < pointsPerStrand.Length;) {
            int iEnd = i;
            int totalPoints = 0;
            while (iEnd < pointsPerStrand.Length && totalPoints + pointsPerStrand[iEnd] < limit)
                totalPoints += pointsPerStrand[iEnd++];

            var hairObject = CreateHairObject("unnameprivated");

            var hairObjectIndexer = hairObject.GetComponent<HairIndexer>();
            hairObjectIndexer.hairBeginIndex = i;
            hairObjectIndexer.particleBeginIndex = pointCur;
            hairObjectIndexer.hairEndIndex = iEnd;
            hairObjectIndexer.particleEndIndex = pointCur + totalPoints;

            var hairObjectMesh = hairObject.GetComponent<MeshFilter>().mesh;

            var vertices = new Vector3[totalPoints];
            var colors = new Color[totalPoints];
            var indices = new int[2 * (totalPoints - (iEnd - i))];
            Color currentColor = Random.ColorHSV(0.0f, 1.0f, 0.6f, 0.6f, 0.7f, 0.7f);
            for (int p = pointCur, ic = 0, lc = i, lcPointCur = 0; p < pointCur + totalPoints; ++p) {
                //p indicates the current scanning position in the "points" array, which means p - pointCur is the current position in the "vertices" array
                //vc indicates the current vertex position, vc = p - pointCur
                //ic indicates the current poistion in the "indices" array
                //lc indices the current scanning position in the lines
                //lcPointCur indices the current point position in lines[lc]
                int vc = p - pointCur;
                vertices[vc] = points[p];
                colors[vc] = currentColor;

                //if it is not the last point in the line
                if (lcPointCur < pointsPerStrand[lc] - 1) {
                    indices[ic] = vc;
                    indices[ic + 1] = vc + 1;
                    ic += 2;
                    ++lcPointCur;
                } 
                else {
                    ++lc;
                    lcPointCur = 0;
                    currentColor = Random.ColorHSV(0.0f, 1.0f, 0.6f, 0.6f, 0.7f, 0.7f);
                }
            }

            hairObjectMesh.vertices = vertices;
            hairObjectMesh.colors = colors;
            hairObjectMesh.SetIndices(indices, MeshTopology.Lines, 0);
            hairObjectMesh.RecalculateBounds();

            ret.Add(hairObject); 

            i = iEnd;
            pointCur += totalPoints;
        }
        return ret;
    }

    public static GameObject CreateHairObject(string name) {
        var ret = new GameObject("sub-hair");
        ret.AddComponent<MeshFilter>();
        ret.AddComponent<HairIndexer>();
        ret.GetComponent<HairIndexer>().hairName = name;
        ret.AddComponent<MeshRenderer>();
        return ret;
    }
}
