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
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");

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

    //===Vẽ đồ thị===

    //lấy dữ liệu từ kho chung
    auto data=DataManager::instance();
    const auto& rawData =DataManager::instance()->getData();
    float minX=data->minX();
    float maxX=data->maxX();
    float minY=data->minY();
    float maxY=data->maxY();
    if(rawData.empty()) return;

    program->bind();
    vao.bind();

    barData.clear();
    float barWidth = 0.03f; // Độ rộng của mỗi cột

    for (const auto &p:rawData) {

        // Mapping sang OpenGL
        float xGL = mapvalue(p.x, minX, maxX, -0.9f, 0.9f);
        float yGL = mapvalue(p.y, minY, maxY, -1.0f, 1.0f);
        float yBottom = -1.0f; // Đáy cột luôn nằm ở cạnh dưới màn hình

        // TẠO 6 ĐỈNH CHO 1 HÌNH CHỮ NHẬT (2 Tam giác)
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

