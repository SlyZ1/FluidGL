ivec3 cellToCoord(int cell, int nx, int ny){
    int column = cell % nx;
    int rest = (cell - column) / nx;
    int row = rest % ny;
    int layer = (rest - row) / ny;
    return ivec3(column, row, layer);
}

int coordToCell(ivec3 coord, int nx, int ny, int nz){
    return clamp(coord.x % nx + nx * (coord.y % ny) + nx * ny * coord.z, 0, nx * ny * nz - 1);
}

int posToCell(vec3 pos, int nx, int ny, int nz){
    ivec3 n = ivec3(nx, ny, nz);
    vec3 floatCoord = floor(pos / h + vec3(n % 2) * 0.5);
    ivec3 coord = ivec3(floatCoord) + ivec3(floor(vec3(n) * 0.5));
    return coordToCell(coord, nx, ny, nz);
}

ivec3 posToCoord(vec3 pos, int nx, int ny, int nz){
    ivec3 n = ivec3(nx, ny, nz);
    vec3 floatCoord = floor(pos / h + vec3(n % 2) * 0.5);
    ivec3 coord = ivec3(floatCoord) + ivec3(floor(vec3(n) * 0.5));
    return coord;
}

vec3 cellToPos(int cell, int nx, int ny, int nz){
    ivec3 coord = cellToCoord(cell, nx, ny);
    vec3 pos = (vec3(coord) - vec3(nx - 1, ny - 1, nz - 1) * 0.5) * h;
    return pos;
}

vec3 coordToPos(ivec3 coord, int nx, int ny, int nz){
    vec3 pos = (vec3(coord) - vec3(nx - 1, ny - 1, nz - 1) * 0.5) * h;
    return pos;
}