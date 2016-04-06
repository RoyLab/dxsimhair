# Guide Hairs, \*.guide, \*.info

int a: number of guide strand  
int a1: particle per strand   
int b: number of frame  
int \* a: guide id  
int: frame id  
float \* a \* a1 \* (6+3): rot, translate  

---

# Weights, \*.weights

int b: number of strand  

For Loop \* b  
int: number of guides  
(int, float):   guide Id, guide weight

---

# Ground Truth, \*.anim

int a: frame number  
int b: particle number  

For Loop \* a  
int: frame id
* For Loop \* b  
* float \* 3: position

# Reference, \*.refs

int b: particle number
float \* 3: position

__似乎没有必要__
