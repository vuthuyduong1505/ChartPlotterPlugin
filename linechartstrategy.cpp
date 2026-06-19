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
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader.frag");
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
float LineChartStrategy::mapvalue(float input, float inputMin, float inputMax, float outputMin, float outputMax)
{
    return (outputMax - outputMin) * (input - inputMin) / (inputMax - inputMin) + outputMin;
}

void LineChartStrategy::draw(QOpenGLFunctions *f, float time)
{
    program->bind();
    vao.bind();
//===VẼ LƯỚI TỪ BỘ NHỚ===
    vboGrid.bind();
    program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
    program->enableAttributeArray(0);
    program->setUniformValue("ourColor",QVector4D(0.2f, 0.2f, 0.2f, 1.0f));
    f->glLineWidth(1.0f);
    f->glDrawArrays(GL_LINES,0,gridData.size()/3);

    // ===Vẽ hệ trục tọa độ===

    vboAxes.bind();
    program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
    program->enableAttributeArray(0);
    program->setUniformValue("ourColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINES,0,4);

    //===Vẽ đồ thị===

    //tạo danh sách điểm mới
    plotData.clear();
    int numPoint =100;
    for(int i=0;i<numPoint;i++)
    {
        float x=static_cast<float>(i); // ép kiểu an toàn sang float để tính toán
        float y= sin(i*0.2f +time)*40 +50;

        // ánh xạ sang hệ tọa độ GL
        float xGL = mapvalue(x,0,99,-1.0f,1.0f);
        float yGL = mapvalue(y,0,100,-1.0f,1.0f);

        plotData.push_back(xGL);
        plotData.push_back(yGL);
        plotData.push_back(0.0f);
    }

    // gửi dữ liệu mới lên GPU
    vboLine.bind();
    vboLine.allocate(plotData.data(), plotData.size()*sizeof(float));
    program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
    program->enableAttributeArray(0);
    // Gửi màu sắc từ c++ vào shader
    //   program->setUniformValue("ourColor", QVector4D(1.0f,1.0f,0.0f,1.0f)); // set màu vàng cho biến ourColor
    // tạo hiệu ứng đổi màu
    float red =0.5f +0.5f* std::sin(time);
    float green = 0.5f +0.5f*std::sin(time+2.0f);
    float blue=0.5f+0.5f*std::sin(time+4.0f);
    program->setUniformValue("ourColor", QVector4D(red, green, blue, 1.0f));

    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINE_STRIP,0,plotData.size()/3);

    vao.release();
    program->release();
}




