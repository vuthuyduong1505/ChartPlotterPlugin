#include "piechartstrategy.h"
#include<cmath>
pieChartStrategy::pieChartStrategy():program(nullptr){}

pieChartStrategy::~pieChartStrategy()
{

}

void pieChartStrategy::init()
{
    program=new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex,":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment,":/shaders/shader.frag");
    program->link();

    vao.create();
    vao.bind();

    vboPie.create();
    vboPie.bind();

    vao.release();
    program->release();
}

void pieChartStrategy::draw(QOpenGLFunctions *f, float time)
{
    program->bind();
    vao.bind();
    // Lấy kích thước cửa sổ và tính tỉ lệ bù trừ
    GLint  viewport[4];
    f->glGetIntegerv(GL_VIEWPORT,viewport);
    int screenWidth =viewport[2];
    int screenHeight =viewport[3];

    float scaleX=1.0f;
    float scaleY=1.0f;

    //nếu màn hình nằm ngang(rộng>cao) -> bóp trục X lại
    if(screenWidth>screenHeight)
    {
        scaleX = static_cast<float>(screenHeight) /screenWidth;
    }
    else {
        scaleY=static_cast<float>(screenWidth)/screenHeight;
    }



    // giả lập dữ liệu 3 phần chiếm tỉ lệ lần lượt là:
    std::vector<float> values={30.0f,50.0f,20.0f};

    // bảng màu tương ứng cho ba phần
    std::vector<QVector4D> colors={
        QVector4D(0.9f, 0.3f, 0.3f, 1.0f), // Màu đỏ nhạt
        QVector4D(0.3f, 0.8f, 0.4f, 1.0f), // Màu xanh lá
        QVector4D(0.2f, 0.5f, 0.9f, 1.0f)  // Màu xanh dương
    };

    float total=0.0f;
    for(float v:values) total=total+v;

    float startAngle=0.0f;
    float radius=0.6f;

    //vòng lặp vẽ từng miếng bánh
    for(size_t i=0;i<values.size();++i)
    {
        // tính góc quét của miếng bánh hiện tại
        float sliceAngle= (values[i]/total)*2.0f*M_PI;
        float endAngle= startAngle+sliceAngle;

        std::vector<float>pieData;
        int segment =100;
        for(int j=0;j<segment;++j)
        {
            // tính góc cụ thể cho nấc hiện tại và nấc kế tiếp
            float aLpha1= startAngle + (static_cast<float>(j)/segment)*sliceAngle;
            float alpha2=startAngle +(static_cast<float>(j+1)/segment)*sliceAngle;

            //Đỉnh 1: tâm hình tròn
            pieData.push_back(0.0f); pieData.push_back(0.0f); pieData.push_back(0.0f);

            //Đỉnh 2: Điểm tại góc alpha1
            pieData.push_back(radius* std::cos(aLpha1)*scaleX);
            pieData.push_back(radius* std::sin(aLpha1)*scaleY);
            pieData.push_back(0.0f);

            //Đỉnh 3:Điểm tại góc alpha2
            pieData.push_back(radius*std::cos(alpha2)*scaleX);
            pieData.push_back(radius*std::sin(alpha2)*scaleY);
            pieData.push_back(0.0f);
        }

        vboPie.bind();
        vboPie.allocate(pieData.data(), pieData.size()*sizeof(float));
        program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
        program->enableAttributeArray(0);

        program->setUniformValue("ourColor",colors[i]);

        f->glDrawArrays(GL_TRIANGLES,0,pieData.size()/3);

        startAngle=endAngle;
    }
    vao.release();
    program->release();


}
