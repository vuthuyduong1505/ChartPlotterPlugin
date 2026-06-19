#include "barchartstrategy.h"

BarChartStrategy::BarChartStrategy(): program(nullptr){}

BarChartStrategy::~BarChartStrategy()
{
    vboBar.destroy();
    vao.destroy();
    delete program;
}

void BarChartStrategy::init()
{
    program=new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader.frag");

    program->bind();
    vao.create();
    vao.bind();
    vboBar.create();
    vboBar.bind();
    program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
    program->enableAttributeArray(0);
    vao.release();

}
float BarChartStrategy::mapvalue(float input, float inputMin, float inputMax, float outputMin, float outputMax)
{
    return (outputMax - outputMin) * (input - inputMin) / (inputMax - inputMin) + outputMin;
}
void BarChartStrategy::draw(QOpenGLFunctions *f, float time)
{
    program->bind();
    vao.bind();
  /*  gridData.clear();
    // vẽ các đường lưới dọc và nagng cách nhau 0.2 đơn vị OpenGL
    for(float i=-1.0f;i<=1.0f;i=i+0.2f)
    {
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
    vbo.bind();
    vbo.allocate(gridData.data(),gridData.size()*sizeof(float));
    program->setUniformValue("ourColor",QVector4D(0.2f, 0.2f, 0.2f, 1.0f));
    f->glLineWidth(1.0f);
    f->glDrawArrays(GL_LINES,0,gridData.size()/3);

    // ===Vẽ hệ trục tọa độ===

    float axesData[]={
        -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // trục X
        0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f // trục Y
    };

    vbo.bind();
    vbo.allocate(axesData, sizeof(axesData));

    program->setUniformValue("ourColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINES,0,4);*/

    //===Vẽ đồ thị===

    //tạo danh sách điểm mới
    barData.clear();
    int numBars = 20; // Vẽ 20 cột cho dễ nhìn
    float barWidth = 0.03f; // Độ rộng của mỗi cột
  //  float gap=0.005f;
    for (int i = 0; i < numBars; ++i) {
        // 1. Tính toán giá trị thực
        float xReal = i * (100.0f / numBars);
        float yReal = std::sin(i * 0.5f + time) * 40 + 50;

        // 2. Mapping sang OpenGL
        float xGL = mapvalue(xReal, 0, 100, -0.9f, 0.9f);
        float yGL = mapvalue(yReal, 0, 100, -1.0f, 1.0f);
        float yBottom = -1.0f; // Đáy cột luôn nằm ở cạnh dưới màn hình

        // 3. TẠO 6 ĐỈNH CHO 1 HÌNH CHỮ NHẬT (2 Tam giác)
        // Tam giác 1
        barData.push_back(xGL - barWidth); barData.push_back(yBottom); barData.push_back(0); // Trái dưới
        barData.push_back(xGL + barWidth); barData.push_back(yBottom); barData.push_back(0); // Phải dưới
        barData.push_back(xGL - barWidth); barData.push_back(yGL);     barData.push_back(0); // Trái trên

        // Tam giác 2
        barData.push_back(xGL - barWidth); barData.push_back(yGL);     barData.push_back(0); // Trái trên
        barData.push_back(xGL + barWidth); barData.push_back(yBottom); barData.push_back(0); // Phải dưới
        barData.push_back(xGL + barWidth); barData.push_back(yGL);     barData.push_back(0); // Phải trên
    }

    // gửi dữ liệu mới lên GPU
    vboBar.bind();
    vboBar.allocate(barData.data(), barData.size()*sizeof(float));


    // Gửi màu sắc từ c++ vào shader
    //   program->setUniformValue("ourColor", QVector4D(1.0f,1.0f,0.0f,1.0f)); // set màu vàng cho biến ourColor
    // tạo hiệu ứng đổi màu
    float red =0.5f +0.5f* std::sin(time);
    float green = 0.5f +0.5f*std::sin(time+2.0f);
    float blue=0.5f+0.5f*std::sin(time+4.0f);
    program->setUniformValue("ourColor", QVector4D(red, green, blue, 1.0f));

    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_TRIANGLES,0,barData.size()/3);

    vao.release();
    program->release();

}

