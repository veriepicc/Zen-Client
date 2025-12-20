export module GLMatrix;

import Paul;

export struct GLMatrix
{
    float matrix[16];

    GLMatrix() {
        for (int i = 0; i < 16; i++) matrix[i] = 0;
    }
};
