#include "linechartstrategy.h"

LineChartStrategy::LineChartStrategy() :program(nullptr){}

LineChartStrategy::~LineChartStrategy()
{
    vboLine.destroy();
    vboAxes.destroy();
    vboGrid.destroy();
    vao.destroy();
    delete program;
}

void LineChartStrategy::init()
{
    program= new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex,  ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->bind();

    vao.create();
    vao.bind();

// ===Vẽ lưới cố định===
    vboGrid.create();
    vboGrid.bind();
    gridData.clear();
    for(int step =0;step<=10;step++)
    {
        float i=-1.0f+step*0.2f;
        // đường dọc
        gridData.push_back(i);
        gridData.push_back(-1.0f);
        gridData.push_back(0.0f);

        gridData.push_back(i);
        gridData.push_back(1.0f);
        gridData.push_back(0.0f);

        // đường ngang
        gridData.push_back(-1.0f);
        gridData.push_back(i);
        gridData.push_back(0.0f);

        gridData.push_back(1.0f);
        gridData.push_back(i);
        gridData.push_back(0.0f);
    }
    vboGrid.allocate(gridData.data(), gridData.size()*sizeof(float));


    //===VẼ TRỤC CỐ ĐỊNH===
    vboAxes.create();
    vboAxes.bind();
    float axesData[]={
        -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // trục X
        0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f // trục Y
    };
    vboAxes.allocate(axesData,sizeof(axesData));

    //== KHỞI TẠO VBO ĐỒ THỊ(LINE)===
    vboLine.create();

    vao.release();
    program->release();
}
void LineChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                             const std::vector<DataPoint> &rawData,
                             float minX, float maxX, float minY, float maxY,
                             bool dataDirty, int lineStyle)
{
    if (rawData.empty()) return;

    // Trục Y bảo vệ chia cho 0
    float renderMinY = minY;
    float renderMaxY = maxY;
    if (std::abs(renderMaxY - renderMinY) < 0.0001f) {
        renderMaxY += 1.0f;
        renderMinY -= 1.0f;
    }

    // Tỷ lệ khoảng đệm 20% cho trục Y
    float paddingY = (renderMaxY - renderMinY) * 0.2f;
    renderMinY -= paddingY;
    renderMaxY += paddingY;

    program->bind();
    vao.bind();

    // 1. Vẽ lưới (Bypass mapping bằng cách đặt u_useMapping = 0)
    program->setUniformValue("u_useMapping", 0);
    program->setUniformValue("u_lineStyle", 0); // Lưới vẽ nét liền
    program->setUniformValue("ourColor", QVector4D(0.2f, 0.2f, 0.2f, 1.0f));
    vboGrid.bind();
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glLineWidth(1.0f);
    f->glDrawArrays(GL_LINES, 0, gridData.size() / 3);

    // 2. Vẽ hệ trục tọa độ (Bypass mapping)
    program->setUniformValue("ourColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    vboAxes.bind();
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINES, 0, 4);

    // 3. Vẽ đồ thị Line (Sử dụng camera động từ minX, maxX)
    // Thiết lập Uniforms để Shader thực hiện ánh xạ
    program->setUniformValue("u_useMapping", 1);
    program->setUniformValue("u_lineStyle", lineStyle); // Áp dụng kiểu nét vẽ của đồ thị
    program->setUniformValue("u_minX", minX);
    program->setUniformValue("u_maxX", maxX);
    program->setUniformValue("u_minY", renderMinY);
    program->setUniformValue("u_maxY", renderMaxY);
    program->setUniformValue("u_mapMinX", -1.0f);
    program->setUniformValue("u_mapMaxX", 1.0f);
    program->setUniformValue("u_mapMinY", -1.0f);
    program->setUniformValue("u_mapMaxY", 1.0f);

    QVector4D glColor(color.redF(), color.greenF(), color.blueF(), 1.0f);
    program->setUniformValue("ourColor", glColor);

    // TỐI ƯU: Chỉ allocate VBO khi dữ liệu thay đổi
    if (dataDirty || !vboLine.isCreated()) {
        plotData.clear();
        plotData.reserve(rawData.size() * 3);
        for (const auto& p : rawData) {
            plotData.push_back(p.x);
            plotData.push_back(p.y);
            plotData.push_back(0.0f);
        }
        if (!vboLine.isCreated()) {
            vboLine.create();
        }
        vboLine.bind();
        vboLine.allocate(plotData.data(), plotData.size() * sizeof(float));
    } else {
        vboLine.bind();
    }

    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);

    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINE_STRIP, 0, plotData.size() / 3);

    vao.release();
    program->release();
}




