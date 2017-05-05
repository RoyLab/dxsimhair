using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

/// <summary>
/// HairLoader:
/// A helper class for loading the hair model from .hair extension file
/// </summary>
public class HairLoader {

    public delegate Color ColorApply();
    public static Color ColorApplyDefault() {
        return Random.ColorHSV(0.0f, 1.0f, 0.6f, 0.6f, 0.7f, 0.7f);
    }

    public delegate int NumberOfParticleAtStrandAtIndex(int index);
  
    public static List<GameObject> Load(Vector3[] particles, int strandCount, NumberOfParticleAtStrandAtIndex numberOfParticleAtStrandAtIndex, ColorApply colorApply = null, int limit=60000, string hairName="unknown") {

        if (colorApply == null)
            colorApply = ColorApplyDefault;

        List<GameObject> ret = new List<GameObject>();
        Color color = colorApply();

        for (int i = 0, pointCur = 0, groupIdx = 0; i < strandCount;)
        {
            //i indicates the current lines we are scanning
            //pointCur indicates the current point cusor we are scanning
            //groupIdx indicates the how many group we have scanned
            //iEnd indicates the group hair strand is in [i, iEnd)
            //totalPoints indicates the total 
            int iEnd = i;
            int totalPoints = 0;
            while (iEnd < strandCount && totalPoints + numberOfParticleAtStrandAtIndex(iEnd) < limit)
                totalPoints += numberOfParticleAtStrandAtIndex(iEnd++);

            //this time create a group hair object
            var hairObject = CreateHairObject(hairName);
            //when creating the group and the color apply frequency is EVERY_GROUP
            //get the color 
            ++groupIdx;

            var hairObjectIndexer = hairObject.GetComponent<HairIndexer>();
            hairObjectIndexer.hairBeginIndex = i;
            hairObjectIndexer.particleBeginIndex = pointCur;
            hairObjectIndexer.hairEndIndex = iEnd;
            hairObjectIndexer.particleEndIndex = pointCur + totalPoints;

            var hairObjectMesh = hairObject.GetComponent<MeshFilter>().mesh;

            var vertices = new Vector3[totalPoints];
            var colors = new Color[totalPoints];
            var indices = new int[2 * (totalPoints - (iEnd - i))];

            for (int p = pointCur, ic = 0, lc = i, lcPointCur = 0; p < pointCur + totalPoints; ++p)
            {
                //p indicates the current scanning position in the "points" array, which means p - pointCur is the current position in the "vertices" array
                //vc indicates the current vertex position, vc = p - pointCur
                //ic indicates the current poistion in the "indices" array
                //lc indices the current scanning position in the lines
                //lcPointCur indices the current point position in lines[lc]
                int vc = p - pointCur;
                vertices[vc] = particles[p];
                colors[vc] = color;

                //if it is not the last point in the line
                if (lcPointCur < numberOfParticleAtStrandAtIndex(lc) - 1)
                {
                    indices[ic] = vc;
                    indices[ic + 1] = vc + 1;
                    ic += 2;
                    ++lcPointCur;
                }
                else
                {
                    ++lc;
                    lcPointCur = 0;
                    color = colorApply();
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

    public static List<GameObject> LoadFixedParticlePerStrand(Vector3[] particles, int strandCount, int particlePerStrandCount)
    {
        return Load(particles, strandCount, delegate (int _) { return particlePerStrandCount; });
    }

    public static List<GameObject> LoadStaticHair(string name, int limit = 60000)
    {
        var asset = Resources.Load(name) as TextAsset;
        using (var reader = new BinaryReader(new MemoryStream(asset.bytes)))
        {
            var particleCount = reader.ReadInt32();
            var particles = new Vector3[particleCount];
            for (int i = 0; i < particleCount; ++i)
                particles[i] = new Vector3(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
            var strandCount = reader.ReadInt32();
            var strandPointsCount = new int[strandCount];
            for (int i = 0; i < strandCount; ++i)
                strandPointsCount[i] = reader.ReadInt32();

            return Load(particles, strandCount, delegate (int i) { return strandPointsCount[i]; });
        }
    }

    static GameObject CreateHairObject(string name) {
        var ret = new GameObject("sub-hair");
        ret.AddComponent<MeshFilter>();
        ret.AddComponent<HairIndexer>();
        ret.GetComponent<HairIndexer>().hairName = name;
        ret.AddComponent<MeshRenderer>();
        return ret;
    }
}
