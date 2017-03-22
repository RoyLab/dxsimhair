using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

/// <summary>
/// A group of hair has an index to show
/// the range of the string and the range of
/// the particle
/// </summary>
class HairIndexer: MonoBehaviour
{
    public int particleBeginIndex, particleEndIndex;
    public int hairBeginIndex, hairEndIndex;
    public string hairName;

    HairIndexer() {
    }
}
