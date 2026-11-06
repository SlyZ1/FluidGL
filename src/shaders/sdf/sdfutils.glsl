int coordToCell(int i, int j, int k){
    return i + j * N.x + k * N.x * N.y;
}

vec3 coordToPos(int i, int j, int k){
    return (vec3(i,j,k) - (vec3(N - 1)) * 0.5) * dx;
}