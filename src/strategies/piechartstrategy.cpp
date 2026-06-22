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
    const auto rawData = DataManager::instance()->getData();
    if(rawData.empty())return;
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



    // Xử  lí dữ liệu thực tế
    std::vector<float> values;

    float total=0.0f;
    for(const auto &p:rawData){
        float val=std::abs(p.y);
        values.push_back(val);
        total+=val;
    }

    if(total==0.0f)
    {
        vao.release();
        program->release();
        return;
    }
    // Bảng màu (Thêm vài màu để đồ thị sinh động hơn)
    std::vector<QVector4D> colors = {
        QVector4D(0.9f, 0.3f, 0.3f, 1.0f), // Đỏ nhạt
        QVector4D(0.3f, 0.8f, 0.4f, 1.0f), // Xanh lá
        QVector4D(0.2f, 0.5f, 0.9f, 1.0f), // Xanh dương
        QVector4D(0.9f, 0.7f, 0.1f, 1.0f), // Vàng
        QVector4D(0.6f, 0.2f, 0.8f, 1.0f)  // Tím
    };
    float startAngle=0.0f;
    float radius=0.6f;

    //vòng lặp vẽ từng miếng bánh
    for(size_t i=0;i<values.size();++i)
    {
        // tính góc quét của miếng bánh hiện tại
        float sliceAngle= (values[i]/total)*2.0f*M_PI;
        float endAngle= startAngle+sliceAngle;

        std::vector<float>pieData;
        int segment =30;
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

        program->setUniformValue("ourColor",colors[i % colors.size()]);

        f->glDrawArrays(GL_TRIANGLES,0,pieData.size()/3);

        startAngle=endAngle;
    }
    vao.release();
    program->release();


}
