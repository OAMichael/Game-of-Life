#version 460 core

layout(location = 0) out vec4 outColor;
layout(location = 1) in  vec2 inUv;

uniform sampler2D ourTexture;


uniform ivec2 gridSize;
uniform ivec2 winSize;

uniform mat4 transformInv;


layout(std430, binding = 0) buffer SSBO_
{
    int totalCells[];
};



int isCellAlive() {
    vec2 coords = (transformInv * vec4( (gl_FragCoord.xy - 0.5) / winSize * 2.0f - 1.0f, 0.0f, 1.0f )).xy / 2.0f + 0.5f;
    ivec2 cellIndex = ivec2(gridSize * coords);

    int cellIndexLin = cellIndex.y * gridSize.x + cellIndex.x;

    if(cellIndexLin >= 0 && cellIndexLin < gridSize.x * gridSize.y)
        return totalCells[cellIndexLin];
    else
        return 0;
};


void main()
{
    vec4 deadColor  = vec4(0.1f, 0.1f, 0.5f, 1.0f);
    vec4 aliveColor = vec4(0.1f, 0.7f, 0.1f, 1.0f);

    vec4 texColor = texture(ourTexture, inUv);

    if(isCellAlive() == 1)
        outColor = aliveColor;
    else
        outColor = deadColor;

    outColor = mix(outColor, texColor, 0.4);
}