/************************************************************************
**                                SEdit
**
**   Copyright (C) 2020 Misaki Design.LLC
**   Copyright (C) 2020 Jun Tajima <tajima@misaki-design.co.jp>
**
**   This file is part of the SEdit, Simulation data Editor for Re:sim
**
**   This software is released under the GNU Lesser General Public
**   License version 3, see LICENSE.
*************************************************************************/


#include <QApplication>
#include <QMessageBox>

#include "graphiccanvas.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#define FONT_SCALE  0.018


GraphicCanvas::GraphicCanvas(QOpenGLWidget *parent) : QOpenGLWidget(parent)
{
    vertexShaderProgramFile   = QString();
    fragmentShaderProgramFile = QString();

    program = NULL;

    QString pathTo = QApplication::applicationDirPath();

    QFile file( pathTo + QString("/SEdit_shader_def.txt"));
    qDebug() << "[GraphicCanvas::GraphicCanvas] shader_def: path = " << pathTo + QString("/SEdit_shader_def.txt");

    if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){
        QTextStream in(&file);

        QString line;
        QStringList divLine;

        line = in.readLine();
        divLine = line.split(";");

        vertexShaderProgramFile = pathTo + QString("/") + QString( divLine[1] ).trimmed();
        qDebug() << "   vertexShaderProgramFile   = " << vertexShaderProgramFile;

        line = in.readLine();
        divLine = line.split(";");

        fragmentShaderProgramFile = pathTo + QString("/") + QString( divLine[1] ).trimmed();
        qDebug() << "   fragmentShaderProgramFile = " << fragmentShaderProgramFile;

        file.close();
    }
    else{
        QMessageBox::warning(this,"Error","Cannot read Shader Setting File: SEdit_shader_def.txt, and exit program.");
        exit(-1);
    }


    X_eye = 0.0;
    Y_eye = 0.0;
    Z_eye = -30.0;

    X_trans = X_eye;
    Y_trans = Y_eye;
    Z_trans = Z_eye;

    cameraYaw   = 0.0;
    cameraPitch = 0.0;
    cameraQuat = QQuaternion(1.0,0.0,0.0,0.0);

    fieldOfView = 45.0;
    aspectRatio = 1.0;
    currentWidth = 800.0;
    currentHeight = 600.0;

    sx = 1.0;
    sy = 1.0;

    road = NULL;

    selectedObj.selObjKind.clear();
    selectedObj.selObjID.clear();

    undoInfo.setUndoInfo = false;
    undoInfo.selObjKind.clear();
    undoInfo.selObjID.clear();
    undoInfo.data.clear();

    rotDir = 0;
    numberKeyPressed = -1;

    laneDrawWidth = 6;

    isOrthogonal = false;

    showMapImageFlag       = true;
    backMapImageFlag       = false;
    showNodesFlag          = true;
    showLanesFlag          = true;
    showTrafficSignalsFlag = true;
    showStopLinesFlag      = true;
    showPedestLaneFlag     = true;
    showStaticObjectFlag   = true;
    showRoadBoundaryFlag   = true;

    showNodeLabelsFlag          = true;
    showLaneLabelsFlag          = true;
    showTrafficSignalLabelsFlag = true;
    showStopLineLabelsFlag      = true;
    showPedestLaneLabelsFlag    = true;
    showStaticObjectLabelsFlag  = true;
    showRoadBoundaryLabelFlag   = true;
    showLabelsFlag              = true;

    selectNodeFlag          = true;
    selectLaneFlag          = true;
    selectTrafficSignalFlag = true;
    selectStopLineFlag      = true;
    selectPedestLaneFlag    = true;
    selectStaticObjectFlag  = true;
    selectRoadBoundaryFlag  = true;

    LaneListFlag           = false;
    laneListIndex          = 0;
    RelatedLanesFlag       = false;
    RouteLaneListFlag      = true;

    colorLaneBySpeedLimitFlag  = false;
    colorLaneByActualSpeedFlag = false;

    nodePickModeFlag = false;
    pedestPathPointPickFlag = false;
    roadBoundaryPointPickFlag = false;

    mousePressed = false;

    scenarioPickMode = 0;
    scenarioPickCount = 0;

    selectByArea = false;
    cutLaneByLine = false;

    addObjToNodePopup = new QMenu();

    createVTS = new QAction();
    createVTS->setText("Traffic Signal for Vehicle");
    addObjToNodePopup->addAction( createVTS );

    createPTS = new QAction();
    createPTS->setText("Traffic Signal for Pedestrian");
    addObjToNodePopup->addAction( createPTS );

    createSL = new QAction();
    createSL->setText("Stop Line");
    addObjToNodePopup->addAction( createSL );

    lastImageLoadFolder = QString();

    dispCtrl = NULL;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    setMouseTracking(true);

    // hasPendingRequest = false;
}


GraphicCanvas::~GraphicCanvas()
{
    qDebug() << "Clean up";

    makeCurrent();
    if( program ){
        program->release();
        delete program;
    }
    doneCurrent();
}



void GraphicCanvas::initializeGL()
{
    bool ret;

    qDebug() << "[GraphicCanvas::initializeGL]";


    //
    //  Initialize OpenGL Functions
    //
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //
    //  Load shaders
    //
    program = new QOpenGLShaderProgram();

    ret = program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderProgramFile);
    if( !ret ){
        qDebug() << "   addShaderFromSourceFile(Vertex) failed.";
    }

    ret = program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderProgramFile);
    if( !ret ){
        qDebug() << "   addShaderFromSourceFile(Fragment) failed.";
    }

    ret = program->link();
    if( !ret ){
        qDebug() << "   program->link failed.";
    }

    ret = program->bind();
    if( !ret ){
        qDebug() << "   program->bind failed.";
    }

    u_modelToCamera = program->uniformLocation("modelToWorld");
    u_CameraToView  = program->uniformLocation("worldToView");

    u_isText        = program->uniformLocation("isText");
    u_letterPos     = program->uniformLocation("letterPos");

    u_useTex        = program->uniformLocation("useTex");
    u_vColor        = program->uniformLocation("vColor");



    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Font
    Characters.clear();

    FT_Library ft;
    if (FT_Init_FreeType(&ft)){
        qDebug() << "   FREETYPE: Could not init FreeType Library";
    }

    FT_Face face;

    QString pathTo = QApplication::applicationDirPath();
    pathTo += QString("/togoshi-mono.TTF");

    if (FT_New_Face(ft, pathTo.toLocal8Bit().data() , 0, &face)){
        qDebug() << "   FREETYPE: Failed to load font, exit program.";
        exit(-1);
    }

    FT_Set_Pixel_Sizes(face, 24, 24);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++){
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            qDebug() << "   FREETYTPE: Failed to load Glyph";
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
            );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Now store character for later use
        Character *character = new Character;
        character->TextureID = texture;
        character->Size.setWidth(face->glyph->bitmap.width );
        character->Size.setHeight( face->glyph->bitmap.rows );
        character->Bearing.setWidth( face->glyph->bitmap_left );
        character->Bearing.setHeight( face->glyph->bitmap_top );
        character->Advance = face->glyph->advance.x * 1.2;

        Characters.insert(c, character);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // prepare for Letters
    Character* ch = Characters[ 'A' ];
    GLfloat w = ch->Size.width();
    GLfloat h = ch->Size.height();

    float scale = FONT_SCALE;
    w *= scale;
    h *= scale;

    QVector<GLfloat> fontPoly;
    fontPoly << 0.0 <<  h   << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
    fontPoly << 0.0 << 0.0  << 0.0 << 0.0 << 1.0 << 1.0 << 1.0 << 1.0;
    fontPoly <<  w  << 0.0  << 0.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
    fontPoly <<  w  <<  h   << 0.0 << 1.0 << 0.0 << 1.0 << 1.0 << 1.0;

    //
    // Axis
    axisPoly.isValid = false;
    ret = axisPoly.array.create();
    if( !ret ){
        qDebug() << "   axis.array.create failed.";
    }
    axisPoly.array.bind();
    axisPoly.buffer = new QOpenGLBuffer();
    ret =  axisPoly.buffer->create();
    if( !ret ){
        qDebug() << "   axis.buffer.create failed.";
    }
    else{
        ret = axisPoly.buffer->bind();
        if( !ret ){
            qDebug() << "   axis.buffer.bind failed.";
        }
        else{
            axisPoly.vertex << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 0.0 << 0.0;
            axisPoly.vertex << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 0.0 << 0.0;
            axisPoly.vertex << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 0.0;
            axisPoly.vertex << 0.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 0.0;
            axisPoly.vertex << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0;
            axisPoly.vertex << 0.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0;

            axisPoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            axisPoly.buffer->allocate( axisPoly.vertex.constData(), axisPoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            axisPoly.buffer->release();
            axisPoly.array.release();

            axisPoly.isValid = true;
        }
    }


    //
    // Text
    textPoly.isTextValid = false;
    ret = textPoly.textArray.create();
    if( !ret ){
        qDebug() << "   textPoly.textArray.create failed.";
    }
    else{

        textPoly.textArray.bind();

        textPoly.textBuffer = new QOpenGLBuffer();
        ret =  textPoly.textBuffer->create();
        if( !ret ){
            qDebug() << "   axis.textBuffer.bind failed.";
        }
        else{
            textPoly.textBuffer->bind();

            textPoly.textBuffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            textPoly.textBuffer->allocate( fontPoly.constData(),
                                       fontPoly.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            textPoly.textBuffer->release();
            textPoly.textArray.release();

            textPoly.isTextValid = true;
        }
    }


    //
    // Circle
    circlePoly.isValid = false;
    ret = circlePoly.array.create();
    if( !ret ){
        qDebug() << "   circlePoly.array.create failed.";
    }
    circlePoly.array.bind();
    circlePoly.buffer = new QOpenGLBuffer();
    ret =  circlePoly.buffer->create();
    if( !ret ){
        qDebug() << "   circlePoly.buffer.create failed.";
    }
    else{
        ret = circlePoly.buffer->bind();
        if( !ret ){
            qDebug() << "   circlePoly.buffer.bind failed.";
        }
        else{

            for(int i=0;i<NODE_CIRCLE_DIV;++i){
                float tht = 2.0 * 3.141592 / NODE_CIRCLE_DIV * (float)i;
                circlePoly.vertex << cos(tht) << sin(tht) << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 0.0;
            }

            circlePoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            circlePoly.buffer->allocate( circlePoly.vertex.constData(), circlePoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            circlePoly.buffer->release();
            circlePoly.array.release();

            circlePoly.isValid = true;
        }
    }


    //
    // Line
    linePoly.isValid = false;
    ret = linePoly.array.create();
    if( !ret ){
        qDebug() << "   linePoly.array.create failed.";
    }
    linePoly.array.bind();
    linePoly.buffer = new QOpenGLBuffer();
    ret =  linePoly.buffer->create();
    if( !ret ){
        qDebug() << "   linePoly.buffer.create failed.";
    }
    else{
        ret = linePoly.buffer->bind();
        if( !ret ){
            qDebug() << "   linePoly.buffer.bind failed.";
        }
        else{

            linePoly.vertex << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            linePoly.vertex << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;

            linePoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            linePoly.buffer->allocate( linePoly.vertex.constData(), linePoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            linePoly.buffer->release();
            linePoly.array.release();

            linePoly.isValid = true;
        }
    }


    //
    // Rectangle
    rectPoly.isValid = false;
    ret = rectPoly.array.create();
    if( !ret ){
        qDebug() << "   rectPoly.array.create failed.";
    }
    rectPoly.array.bind();
    rectPoly.buffer = new QOpenGLBuffer();
    ret =  rectPoly.buffer->create();
    if( !ret ){
        qDebug() << "   rectPoly.buffer.create failed.";
    }
    else{
        ret = rectPoly.buffer->bind();
        if( !ret ){
            qDebug() << "   rectPoly.buffer.bind failed.";
        }
        else{

            rectPoly.vertex << -1.0 << 1.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            rectPoly.vertex << -1.0 << -1.0 << 0.0 << 1.0 << 0.0 << 1.0 << 1.0 << 1.0;
            rectPoly.vertex <<  1.0 << -1.0 << 0.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
            rectPoly.vertex <<  1.0 <<  1.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0 << 1.0;

            rectPoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            rectPoly.buffer->allocate( rectPoly.vertex.constData(), rectPoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            rectPoly.buffer->release();
            rectPoly.array.release();

            rectPoly.isValid = true;
        }
    }


    //
    // Triangle
    trianglePoly.isValid = false;
    ret = trianglePoly.array.create();
    if( !ret ){
        qDebug() << "   trianglePoly.array.create failed.";
    }
    trianglePoly.array.bind();
    trianglePoly.buffer = new QOpenGLBuffer();
    ret =  trianglePoly.buffer->create();
    if( !ret ){
        qDebug() << "   trianglePoly.buffer.create failed.";
    }
    else{
        ret = trianglePoly.buffer->bind();
        if( !ret ){
            qDebug() << "   trianglePoly.buffer.bind failed.";
        }
        else{

            trianglePoly.vertex <<  1.0 <<    0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            trianglePoly.vertex << -0.5 <<  0.707 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            trianglePoly.vertex << -0.5 << -0.707 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;

            trianglePoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            trianglePoly.buffer->allocate( trianglePoly.vertex.constData(), trianglePoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            trianglePoly.buffer->release();
            trianglePoly.array.release();

            trianglePoly.isValid = true;
        }
    }


    // Box
    boxPoly.isValid = false;
    ret = boxPoly.array.create();
    if( !ret ){
        qDebug() << "   boxPoly.array.create failed.";
    }
    boxPoly.array.bind();
    boxPoly.buffer = new QOpenGLBuffer();
    ret = boxPoly.buffer->create();
    if( !ret ){
        qDebug() << "   boxPoly.buffer.create failed.";
    }
    else{
        ret = boxPoly.buffer->bind();
        if( !ret ){
            qDebug() << "   boxPoly.buffer.bind failed.";
        }
        else{

            float x[8];
            float y[8];
            float z[8];

            x[0] =  1.0;   y[0] =  1.0;  z[0] = 0.0;
            x[1] = -1.0;   y[1] =  1.0;  z[1] = 0.0;
            x[2] = -1.0;   y[2] =  1.0;  z[2] = 1.0;
            x[3] =  1.0;   y[3] =  1.0;  z[3] = 1.0;
            x[4] =  1.0;   y[4] = -1.0;  z[4] = 0.0;
            x[5] = -1.0;   y[5] = -1.0;  z[5] = 0.0;
            x[6] = -1.0;   y[6] = -1.0;  z[6] = 1.0;
            x[7] =  1.0;   y[7] = -1.0;  z[7] = 1.0;


            //left
            boxPoly.vertex << x[3] << y[3] << z[3] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[0] << y[0] << z[0] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[1] << y[1] << z[1] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[2] << y[2] << z[2] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;


            // top
            boxPoly.vertex << x[6] << y[6] << z[6] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[7] << y[7] << z[7] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[3] << y[3] << z[3] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[2] << y[2] << z[2] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;


            //right
            boxPoly.vertex << x[6] << y[6] << z[6] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[5] << y[5] << z[5] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[4] << y[4] << z[4] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[7] << y[7] << z[7] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;


            // front
            boxPoly.vertex << x[3] << y[3] << z[3] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[7] << y[7] << z[7] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[4] << y[4] << z[4] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[0] << y[0] << z[0] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;


            // back
            boxPoly.vertex << x[1] << y[1] << z[1] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[5] << y[5] << z[5] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[6] << y[6] << z[6] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;
            boxPoly.vertex << x[2] << y[2] << z[2] << 0.0f << 0.0f << 0.2f << 0.2f << 0.7f;



            boxPoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            boxPoly.buffer->allocate( boxPoly.vertex.constData(), boxPoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            boxPoly.buffer->release();
            boxPoly.array.release();

            boxPoly.isValid = true;
        }
    }


    //
    // Vehicle
    vhclPoly.isValid = false;
    ret = vhclPoly.array.create();
    if( !ret ){
        qDebug() << "   vhclPoly.array.create failed.";
    }
    vhclPoly.array.bind();
    vhclPoly.buffer = new QOpenGLBuffer();
    ret =  vhclPoly.buffer->create();
    if( !ret ){
        qDebug() << "   vhclPoly.buffer.create failed.";
    }
    else{
        ret = vhclPoly.buffer->bind();
        if( !ret ){
            qDebug() << "   vhclPoly.buffer.bind failed.";
        }
        else{

            vhclPoly.vertex <<  1.0  <<  0.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            vhclPoly.vertex <<  0.75 <<  1.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            vhclPoly.vertex << -1.0  <<  1.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            vhclPoly.vertex << -1.0  << -1.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;
            vhclPoly.vertex <<  0.75 << -1.0 << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 1.0;

            vhclPoly.buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
            vhclPoly.buffer->allocate( vhclPoly.vertex.constData(), vhclPoly.vertex.size() * sizeof(GLfloat) );

            program->enableAttributeArray( 0 );
            program->setAttributeBuffer( 0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 1 );
            program->setAttributeBuffer( 1, GL_FLOAT, 3 * sizeof(GLfloat) , 2, 8 * sizeof(GLfloat) );

            program->enableAttributeArray( 2 );
            program->setAttributeBuffer( 2, GL_FLOAT, 5 * sizeof(GLfloat) , 3, 8 * sizeof(GLfloat) );

            vhclPoly.buffer->release();
            vhclPoly.array.release();

            vhclPoly.isValid = true;
        }
    }
}



void GraphicCanvas::paintGL()
{
    makeCurrent();

    // Restrict Frame Rate 20FPS => 1000/20=50[msec]

    // QueryPerformanceCounter(&end);
    // double calTime = static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;  // [msec]
    // if( calTime < 100.0 ){
    //     glClearColor( 0.0, 0.0, 0.0, 1.0 );
    //     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //     // hasPendingRequest = true;
    //     return;
    // }
    // start = end;

    // // hasPendingRequest = false;


    if( nodePickModeFlag == true || pedestPathPointPickFlag == true || roadBoundaryPointPickFlag == true ){
        glClearColor( 0.3451, 0.3843, 0.4336, 1.0 );
    }
    else{
        glClearColor( 0.0, 0.0, 0.0, 1.0 );
    }

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    QMatrix4x4 world2camera;
    world2camera.setToIdentity();

    float xE = X_eye;
    float yE = Y_eye;

    float cpc = cos( cameraPitch );
    float cps = sin( cameraPitch );

    float cyc = cos( cameraYaw );
    float cys = sin( cameraYaw );

    X_trans = xE * cyc - yE * cys;
    Y_trans = xE * cys + yE * cyc;

    yE = Y_trans;
    Y_trans = yE * cpc;
    Z_trans = yE * cps;

    Z_trans += Z_eye;


    world2camera.translate( QVector3D(X_trans,Y_trans, Z_trans) );

    world2camera.rotate( cameraQuat );




    program->setUniformValue( u_CameraToView, projection );

    model2World.setScale( QVector3D(1.0, 1.0, 1.0) );


    //
    // Base Map
    if( mapImageMng != NULL && rectPoly.isValid == true && showMapImageFlag == true ){

        glLineWidth(1.0);

        for(int i=0;i<mapImageMng->baseMapImages.size();++i){

            if( mapImageMng->baseMapImages[i]->isValid == false ){
                continue;
            }

            rectPoly.array.bind();

            float mapZ = -1.0;
            if( backMapImageFlag == true ){
                mapZ = -25.0;
                if( dispCtrl ){
                    mapZ = dispCtrl->backMapOffsetVal->value() * (-1.0);
                }
            }
            model2World.setTranslation( QVector3D( mapImageMng->baseMapImages[i]->x,
                                                   mapImageMng->baseMapImages[i]->y,
                                                   mapZ) );

            float angle = mapImageMng->baseMapImages[i]->rotate * 0.017452;
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0, 0.0, sin(angle*0.5) ) );

            float s = mapImageMng->baseMapImages[i]->scale;
            model2World.setScale( QVector3D(s * mapImageMng->baseMapImages[i]->halfWidth, s * mapImageMng->baseMapImages[i]->halfHeight, 1.0) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_useTex, 1 );
            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

            glActiveTexture( GL_TEXTURE0 );

            glBindTexture(GL_TEXTURE_2D, mapImageMng->baseMapImages[i]->textureID);

            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

            rectPoly.array.release();
        }
    }

    model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

    // RoadBoundary
    if( roadBoundaryPointPickFlag == true && linePoly.isValid == true ){

        model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

        linePoly.array.bind();

        glLineWidth(4.0);

        program->setUniformValue( u_useTex, 2 );
        program->setUniformValue( u_isText, 0 );
        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.5, 1.0, 1.0  ) );

        for(int i=0;i<roadBoundaryPoints.size()-1;++i){

            model2World.setTranslation( QVector3D( roadBoundaryPoints[i]->x(),
                                                   roadBoundaryPoints[i]->y(),
                                                   0.5) );

            float dx = roadBoundaryPoints[i+1]->x() - roadBoundaryPoints[i]->x();
            float dy = roadBoundaryPoints[i+1]->y() - roadBoundaryPoints[i]->y();

            float len = sqrt( dx * dx + dy * dy );
            model2World.setScale( QVector3D(len,1.0,1.0) );

            float angle = atan2( dy, dx );
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            glDrawArrays(GL_LINES, 0, 2 );
        }

        linePoly.array.release();

        if( circlePoly.isValid == true && roadBoundaryPoints.size() > 0 ){

            circlePoly.array.bind();

            model2World.setTranslation( QVector3D( roadBoundaryPoints[0]->x(),
                                                   roadBoundaryPoints[0]->y(),
                                                   0.5) );


            model2World.setRotation( QQuaternion( 1.0, 0.0 , 0.0 , 0.0 )  );

            float w = 1.0;
            model2World.setScale( QVector3D(w,w,1.0) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

            glLineWidth(1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

            circlePoly.array.release();
        }
    }

    if( rectPoly.isValid == true && road != NULL && showRoadBoundaryFlag == true ){

        rectPoly.array.bind();

        for(int i=0;i<road->roadBoundary.size();++i){

            bool isSelected = false;
            if( selectedObj.selObjKind.size() > 0){
                for(int k=0;k<selectedObj.selObjKind.size();++k){
                    if( selectedObj.selObjKind[k] == _SEL_OBJ::SEL_ROAD_BOUNDARY ){
                        if( selectedObj.selObjID[k] == road->roadBoundary[i]->id ){
                            isSelected = true;
                        }
                    }
                }
            }

            for(int j=0;j<road->roadBoundary[i]->pos.size()-1;++j){

                float xc = road->roadBoundary[i]->pos[j]->x();
                float yc = road->roadBoundary[i]->pos[j]->y();
                float zc = road->roadBoundary[i]->pos[j]->z();

                float ex = road->roadBoundary[i]->diff[j]->x() / road->roadBoundary[i]->length[j];
                float ey = road->roadBoundary[i]->diff[j]->y() / road->roadBoundary[i]->length[j];

                float s = road->roadBoundary[i]->length[j] * 0.5;

                xc += ex * s;
                yc += ey * s;

                bool isSelectedSect = false;
                if( selectedObj.selObjKind.size() > 0){
                    for(int k=0;k<selectedObj.selObjKind.size();++k){
                        if( selectedObj.selObjKind[k] == _SEL_OBJ::SEL_ROAD_BOUNDARY_POINT ){
                            if( selectedObj.selObjID[k] == road->roadBoundary[i]->id * 100 + j ){
                                isSelectedSect = true;
                            }
                            else if( j == road->roadBoundary[i]->pos.size() - 2 &&
                                     selectedObj.selObjID[k] == road->roadBoundary[i]->id * 100 + j + 1){
                                isSelectedSect = true;
                            }
                        }
                    }
                }

                {
                    float xd = xc;
                    float yd = yc;
                    float zd = zc;
                    if( road->roadBoundary[i]->RoadSide == 0 ){  // right side is road
                        xd -= 0.5 * ey;
                        yd += 0.5 * ex;
                    }
                    else{
                        xd += 0.5 * ey;
                        yd -= 0.5 * ex;
                    }

                    zd += road->roadBoundary[i]->height[j];

                    model2World.setTranslation( QVector3D( xd, yd, zd) );

                    float angle = road->roadBoundary[i]->angles[j];
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0, 0.0, sin(angle*0.5) ) );

                    model2World.setScale( QVector3D(s, 0.5, 1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    program->setUniformValue( u_isText, 0 );
                    program->setUniformValue( u_useTex, 2 );
                    program->setUniformValue( u_vColor, QVector4D( 0.30, 0.30, 0.30, 1.0 ) );

                    glLineWidth(1.0);
                    glDrawArrays(GL_QUADS, 0, 4 );


                    if( isSelected == true || isSelectedSect == true ){
                        glLineWidth(3.0);
                        program->setUniformValue( u_vColor, QVector4D(1.0, 0.0, 0.0, 1.0) );
                        glDrawArrays(GL_LINE_STRIP, 0, 4 );
                    }

                }

                {
                    float xd = xc;
                    float yd = yc;
                    float zd = zc;
                    float roll = 1.57068;
                    if( road->roadBoundary[i]->RoadSide == 1 ){  // left side is road
                        roll = -1.57068;
                    }

                    float w = road->roadBoundary[i]->height[j] * 0.5;
                    zd += w;

                    model2World.setTranslation( QVector3D( xd, yd, zd) );

                    float angle = road->roadBoundary[i]->angles[j];
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0, 0.0, sin(angle*0.5) ) * QQuaternion( cos(roll*0.5), sin(roll*0.5), 0.0, 0.0 ) );

                    model2World.setScale( QVector3D(s, w, 1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    program->setUniformValue( u_isText, 0 );
                    program->setUniformValue( u_useTex, 2 );
                    program->setUniformValue( u_vColor, QVector4D( 0.40, 0.40, 0.40, 1.0 ) );

                    glLineWidth(1.0);
                    glDrawArrays(GL_QUADS, 0, 4 );
                }

                {
                    float xd = xc;
                    float yd = yc;
                    float zd = zc;
                    if( road->roadBoundary[i]->RoadSide == 0 ){  // right side is road
                        xd += 0.5 * ey;
                        yd -= 0.5 * ex;
                    }
                    else{
                        xd -= 0.5 * ey;
                        yd += 0.5 * ex;
                    }

                    model2World.setTranslation( QVector3D( xd, yd, zd) );

                    float angle = road->roadBoundary[i]->angles[j];
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0, 0.0, sin(angle*0.5) ) );

                    model2World.setScale( QVector3D(s, 0.5, 1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    program->setUniformValue( u_isText, 0 );
                    program->setUniformValue( u_useTex, 2 );
                    program->setUniformValue( u_vColor, QVector4D( 0.50, 0.50, 0.50, 1.0 ) );

                    glLineWidth(1.0);
                    glDrawArrays(GL_QUADS, 0, 4 );

                    if( isSelected == true  || isSelectedSect == true ){
                        glLineWidth(3.0);
                        program->setUniformValue( u_vColor, QVector4D(1.0, 0.0, 0.0, 1.0) );
                        glDrawArrays(GL_LINE_STRIP, 0, 4 );
                    }
                }
            }
        }

        rectPoly.array.release();

        if( textPoly.isTextValid == true && showLabelsFlag == true && showRoadBoundaryLabelFlag == true ){

            textPoly.textArray.bind();
            textPoly.textBuffer->bind();

            program->setUniformValue( u_useTex, 100 );
            program->setUniformValue( u_isText, 100 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

            for(int i=0;i<road->roadBoundary.size();++i){

                char str[50];
                sprintf(str,"RB[%d]",road->roadBoundary[i]->id);

                int j= road->roadBoundary[i]->pos.size() / 2;
                float xc = road->roadBoundary[i]->pos[j]->x();
                float yc = road->roadBoundary[i]->pos[j]->y();
                float zc = road->roadBoundary[i]->pos[j]->z();
                if( j - 1 >= 0 ){
                    zc += road->roadBoundary[i]->height[j-1] + 0.5;
                }
                model2World.setTranslation( QVector3D( xc,yc,zc) );

                QQuaternion letterQuat = cameraQuat.conjugated();
                model2World.setRotation( letterQuat );
                model2World.setScale( QVector3D(1.0,1.0,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glActiveTexture( GL_TEXTURE0 );

                float x = 0.0;
                float y = 0.0;
                float scale = FONT_SCALE;

                for(unsigned int c=0;c<strlen(str);++c ){

                    Character* ch = Characters[ str[c] ];

                    GLfloat xpos = x + ch->Bearing.width() * scale;
                    GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                    program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                    float w = ch->Size.width() * scale;
                    float h = ch->Size.height() * scale;

                    QVector<GLfloat> fontPoly;
                    fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                    fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                    fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                    fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                    textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                    glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                    glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                    x += ( ch->Advance >> 6 ) * scale;
                }
            }

            textPoly.textBuffer->release();
            textPoly.textArray.release();
        }
    }

    // Static Object
    if( boxPoly.isValid == true && road != NULL && showStaticObjectFlag == true ){

        for(int i=0;i<road->staticObj.size();++i){

            boxPoly.array.bind();

            model2World.setTranslation( QVector3D( road->staticObj[i]->xc, road->staticObj[i]->yc, road->staticObj[i]->zc) );

            float angle = road->staticObj[i]->direction * 0.017452;
            model2World.setRotation( QQuaternion( cos(angle), 0.0, 0.0, sin(angle) ) );

            float sx = road->staticObj[i]->lenx;
            float sy = road->staticObj[i]->leny;
            float sz = road->staticObj[i]->height;
            model2World.setScale( QVector3D(sx,sy,sz) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 0.65, 0.65, 0.65, 1.0 ) );

            glLineWidth(1.0);
            glDrawArrays(GL_QUADS, 0, boxPoly.vertex.size() / 8 );

            bool isSelected = false;
            if( selectedObj.selObjKind.size() > 0){
                if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_STATIC_OBJECT ){
                    if( selectedObj.selObjID[0] == road->staticObj[i]->id ){
                        isSelected = true;
                    }
                }
            }

            if( isSelected == true ){
                glLineWidth(3.0);
                program->setUniformValue( u_vColor, QVector4D(1.0, 0.0, 0.0, 1.0) );
            }
            else{
                glLineWidth(1.0);
                program->setUniformValue( u_vColor, QVector4D(1.0, 1.0, 1.0, 1.0) );

            }
            glDrawArrays(GL_LINE_STRIP, 0, boxPoly.vertex.size() / 8 );

            boxPoly.array.release();

            if( textPoly.isTextValid == true && showLabelsFlag == true && showStaticObjectLabelsFlag == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                char str[50];
                sprintf(str,"SO[%d]",road->staticObj[i]->id);

                model2World.setTranslation( QVector3D( road->staticObj[i]->xc,
                                                       road->staticObj[i]->yc,
                                                       road->staticObj[i]->zc + road->staticObj[i]->height + 1.0) );

                QQuaternion letterQuat = cameraQuat.conjugated();
                model2World.setRotation( letterQuat );
                model2World.setScale( QVector3D(1.0,1.0,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glActiveTexture( GL_TEXTURE0 );

                float x = 0.0;
                float y = 0.0;
                float scale = FONT_SCALE;

                for(unsigned int c=0;c<strlen(str);++c ){

                    Character* ch = Characters[ str[c] ];

                    GLfloat xpos = x + ch->Bearing.width() * scale;
                    GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                    program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                    float w = ch->Size.width() * scale;
                    float h = ch->Size.height() * scale;

                    QVector<GLfloat> fontPoly;
                    fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                    fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                    fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                    fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                    textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                    glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                    glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                    x += ( ch->Advance >> 6 ) * scale;
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }
        }

    }


    // WP
    if( circlePoly.isValid == true && road != NULL && showLanesFlag == true ){

        for(int i=0;i<road->nodes.size();++i){

            for(int j=0;j<road->nodes[i]->legInfo.size();++j){

                for(int k=0;k<road->nodes[i]->legInfo[j]->inWPs.size();++k){

                    int wpIdx = road->indexOfWP( road->nodes[i]->legInfo[j]->inWPs[k] );
                    if( wpIdx >= 0 ){

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( road->wps[wpIdx]->pos.x(),
                                                               road->wps[wpIdx]->pos.y(),
                                                               road->wps[wpIdx]->pos.z()) );

                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                        model2World.setScale( QVector3D(0.30,0.30,0.30) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_vColor, QVector4D( 0.8, 0.3, 0.1, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }

                }

                for(int k=0;k<road->nodes[i]->legInfo[j]->outWPs.size();++k){

                    int wpIdx = road->indexOfWP( road->nodes[i]->legInfo[j]->outWPs[k] );
                    if( wpIdx >= 0 ){

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( road->wps[wpIdx]->pos.x(),
                                                               road->wps[wpIdx]->pos.y(),
                                                               road->wps[wpIdx]->pos.z()) );

                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                        model2World.setScale( QVector3D(0.30,0.30,0.30) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_vColor, QVector4D( 0.1, 0.3, 0.8, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }

                }

            }
        }
    }

    model2World.setScale( QVector3D(1.0, 1.0, 1.0) );


    // Cut Lane By Line
    if( cutLaneByLine == true ){

        float dx = cutLaneByLinePoints[1].x() - cutLaneByLinePoints[0].x();
        float dy = cutLaneByLinePoints[1].y() - cutLaneByLinePoints[0].y();
        float len = sqrt( dx * dx + dy * dy );
        if( len > 0.1 ){
            dx /= len;
            dy /= len;

            linePoly.array.bind();

            model2World.setTranslation( QVector3D( cutLaneByLinePoints[0].x(),
                                                   cutLaneByLinePoints[0].y(),
                                                   0.55 ) );

            model2World.setScale( QVector3D(len,1.0,1.0) );

            float angle = atan2( dy, dx );
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

            glLineWidth(4.0);
            glDrawArrays(GL_LINES, 0, 2 );

            linePoly.array.release();
        }
    }

    // Selection Area
    if( selectByArea == true ){

        for(int i=0;i<4;++i){

            linePoly.array.bind();

            float dx = selectByArearPoints[(i+1)%4].x() - selectByArearPoints[i].x();
            float dy = selectByArearPoints[(i+1)%4].y() - selectByArearPoints[i].y();
            float len = sqrt( dx * dx + dy * dy );
            if( len < 0.1 ){
                continue;
            }
            dx /= len;
            dy /= len;

            model2World.setTranslation( QVector3D( selectByArearPoints[i].x(),
                                                   selectByArearPoints[i].y(),
                                                   0.55 ) );

            model2World.setScale( QVector3D(len,1.0,1.0) );

            float angle = atan2( dy, dx );
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

            glLineWidth(4.0);
            glDrawArrays(GL_LINES, 0, 2 );

            linePoly.array.release();
        }

    }

    //
    // Scenario Objects
    if( scenarioPickMode > 0 && scenarioPickCount >= 1 && circlePoly.isValid == true ){

        if( scenarioPickMode == 6 || scenarioPickMode == 9 || scenarioPickMode == 10 ){ // path route
            for(int i=0;i<pathRoutePointStock.size();++i){

                if( i > 0 ){
                    linePoly.array.bind();

                    float dx = pathRoutePointStock[i].x() - pathRoutePointStock[i-1].x();
                    float dy = pathRoutePointStock[i].y() - pathRoutePointStock[i-1].y();
                    float rot = atan2( dy, dx ) * 0.5;
                    float len = sqrt( dx * dx + dy * dy );

                    model2World.setTranslation( QVector3D( pathRoutePointStock[i-1].x(),
                                                           pathRoutePointStock[i-1].y(),
                                                           0.55 ) );
                    model2World.setScale( QVector3D(len,1.0,1.0) );
                    model2World.setRotation( QQuaternion( cos(rot), 0.0 , 0.0 , sin(rot) ) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                    glLineWidth(4.0);
                    glDrawArrays(GL_LINES, 0, 2 );

                    linePoly.array.release();
                }

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( pathRoutePointStock[i].x(),
                                                       pathRoutePointStock[i].y(),
                                                       0.5) );

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(0.30,0.30,0.30) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.1, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();
            }
        }
        else{

            if( scenarioPickCount == 1 ){
                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( pickedPoint1.x(),
                                                       pickedPoint1.y(),
                                                       0.5) );

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(0.30,0.30,0.30) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.1, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();
            }

        }

    }

    for(int i=0;i<scnrEdit->sSys.size();++i){

        if( scnrEdit->sSys[i]->sItem.cond.positionTrigger == true ){

            float xtrigger = scnrEdit->sSys[i]->sItem.cond.ptX;
            float ytrigger = scnrEdit->sSys[i]->sItem.cond.ptY;
            float rottrigger = scnrEdit->sSys[i]->sItem.cond.ptPassAngle * 0.017452 * 0.5;
            float halfWidthTrigger = scnrEdit->sSys[i]->sItem.cond.ptWidth * 0.5;

            trianglePoly.array.bind();

            model2World.setTranslation( QVector3D( xtrigger,ytrigger,0.55) );
            model2World.setRotation( QQuaternion( cos( rottrigger ), 0.0, 0.0, sin( rottrigger ) ) );
            model2World.setScale( QVector3D(0.5, 0.5, 0.5) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

            glLineWidth(1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3 );

            trianglePoly.array.release();

            linePoly.array.bind();

            rottrigger *= 2.0;
            float c = cos( rottrigger );
            float s = sin( rottrigger );

            model2World.setTranslation( QVector3D( xtrigger - s * halfWidthTrigger,
                                                   ytrigger + c * halfWidthTrigger,
                                                   0.55 ) );

            model2World.setScale( QVector3D(halfWidthTrigger * 2.0,1.0,1.0) );

            float angle = atan2( -c, s );
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

            glLineWidth(4.0);
            glDrawArrays(GL_LINES, 0, 2 );

            linePoly.array.release();

            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                char act[50];
                if( scnrEdit->sSys[i]->sItem.act.actionType == 0 ){
                    sprintf(act,"TELEPORT/OBJ[%d]",scnrEdit->sSys[i]->sItem.act.iParams[0]);
                }
                else if( scnrEdit->sSys[i]->sItem.act.actionType == 1 ){
                    sprintf(act,"CHANGE TS/TS[%d]",scnrEdit->sSys[i]->sItem.act.iParams[0]);
                }
                else if( scnrEdit->sSys[i]->sItem.act.actionType == 2 ){
                    sprintf(act,"CHANGE SPEED");
                }
                else if( scnrEdit->sSys[i]->sItem.act.actionType == 3 ){
                    sprintf(act,"SEND UDP");
                }
                else{
                    memset( act, 0, 50 );
                }



                char str[50];
                sprintf(str,"SYS[%d] POS/TRIG TARGET[%d] %s",scnrEdit->sSys[i]->ID, scnrEdit->sSys[i]->sItem.cond.ptTargetObjID, act);

                model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                       ytrigger + 0.5,
                                                       0.60) );

                QQuaternion letterQuat = cameraQuat.conjugated();
                model2World.setRotation( letterQuat );
                model2World.setScale( QVector3D(1.0,1.0,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glActiveTexture( GL_TEXTURE0 );

                float x = 0.0;
                float y = 0.0;
                float scale = FONT_SCALE;

                if( showLabelsFlag == true ){
                    for(unsigned int c=0;c<strlen(str);++c ){

                        Character* ch = Characters[ str[c] ];

                        GLfloat xpos = x + ch->Bearing.width() * scale;
                        GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                        program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                        float w = ch->Size.width() * scale;
                        float h = ch->Size.height() * scale;

                        QVector<GLfloat> fontPoly;
                        fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                        fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                        textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                        glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                        glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                        x += ( ch->Advance >> 6 ) * scale;
                    }
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }
        }

        if( scnrEdit->sSys[i]->sItem.cond.TTCTrigger == true && scnrEdit->sSys[i]->sItem.cond.ttcCalType == 0 ){

            float xtrigger = scnrEdit->sSys[i]->sItem.cond.ttcCalPosX;
            float ytrigger = scnrEdit->sSys[i]->sItem.cond.ttcCalPosY;

            circlePoly.array.bind();

            model2World.setTranslation( QVector3D( xtrigger, ytrigger, 0.55) );
            model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
            model2World.setScale( QVector3D(0.50,0.50,0.50) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

            glLineWidth(1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

            circlePoly.array.release();

            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                char str[50];
                sprintf(str,"SYS[%d] TTC/TRIG/POINT",scnrEdit->sSys[i]->ID);

                model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                       ytrigger + 0.5,
                                                       0.60) );

                QQuaternion letterQuat = cameraQuat.conjugated();
                model2World.setRotation( letterQuat );
                model2World.setScale( QVector3D(1.0,1.0,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glActiveTexture( GL_TEXTURE0 );

                float x = 0.0;
                float y = 0.0;
                float scale = FONT_SCALE;

                if( showLabelsFlag == true ){
                    for(unsigned int c=0;c<strlen(str);++c ){

                        Character* ch = Characters[ str[c] ];

                        GLfloat xpos = x + ch->Bearing.width() * scale;
                        GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                        program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                        float w = ch->Size.width() * scale;
                        float h = ch->Size.height() * scale;

                        QVector<GLfloat> fontPoly;
                        fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                        fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                        textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                        glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                        glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                        x += ( ch->Advance >> 6 ) * scale;
                    }
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }
        }

        if( scnrEdit->sSys[i]->sItem.act.actionType == 0 && scnrEdit->sSys[i]->sItem.act.fParams.size() > 0 ){ // Teleport

            float xTeleportTo = scnrEdit->sSys[i]->sItem.act.fParams[0];
            float yTeleportTo = scnrEdit->sSys[i]->sItem.act.fParams[1];
            float rotTeleportTo = scnrEdit->sSys[i]->sItem.act.fParams[2] * 0.017452 * 0.5;

            trianglePoly.array.bind();

            model2World.setTranslation( QVector3D( xTeleportTo,yTeleportTo,0.55) );
            model2World.setRotation( QQuaternion( cos( rotTeleportTo ), 0.0, 0.0, sin( rotTeleportTo ) ) );
            model2World.setScale( QVector3D(0.5, 0.5, 0.5) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 0.0, 1.0 ) );

            glLineWidth(1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3 );

            trianglePoly.array.release();

            linePoly.array.bind();

            rotTeleportTo *= 2.0;
            float c = cos( rotTeleportTo );
            float s = sin( rotTeleportTo );

            model2World.setTranslation( QVector3D( xTeleportTo - s * 1.5,
                                                   yTeleportTo + c * 1.5,
                                                   0.55 ) );

            model2World.setScale( QVector3D(3.0,1.0,1.0) );

            float angle = atan2( -c, s );
            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
            program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 0.0, 1.0 ) );

            glLineWidth(4.0);
            glDrawArrays(GL_LINES, 0, 2 );

            linePoly.array.release();

            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                char str[50];
                sprintf(str,"SYS[%d] TELEPORT POINT/OBJ[%d]",scnrEdit->sSys[i]->ID,scnrEdit->sSys[i]->sItem.act.iParams[0]);

                model2World.setTranslation( QVector3D( xTeleportTo + 0.5,
                                                       yTeleportTo + 0.5,
                                                       0.60) );

                QQuaternion letterQuat = cameraQuat.conjugated();
                model2World.setRotation( letterQuat );
                model2World.setScale( QVector3D(1.0,1.0,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glActiveTexture( GL_TEXTURE0 );

                float x = 0.0;
                float y = 0.0;
                float scale = FONT_SCALE;

                if( showLabelsFlag == true ){
                    for(unsigned int c=0;c<strlen(str);++c ){

                        Character* ch = Characters[ str[c] ];

                        GLfloat xpos = x + ch->Bearing.width() * scale;
                        GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                        program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                        float w = ch->Size.width() * scale;
                        float h = ch->Size.height() * scale;

                        QVector<GLfloat> fontPoly;
                        fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                        fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                        fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                        textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                        glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                        glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                        x += ( ch->Advance >> 6 ) * scale;
                    }
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }

        }

    }

    for(int i=0;i<scnrEdit->sVehicle.size();++i){

        for(int j=0;j<scnrEdit->sVehicle[i]->sItem.size();++j){

            if( scnrEdit->sVehicle[i]->sItem[j]->cond.positionTrigger == true ){

                float xtrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ptX;
                float ytrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ptY;
                float rottrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ptPassAngle * 0.017452 * 0.5;
                float halfWidthTrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ptWidth * 0.5;

                trianglePoly.array.bind();

                model2World.setTranslation( QVector3D( xtrigger,ytrigger,0.55) );
                model2World.setRotation( QQuaternion( cos( rottrigger ), 0.0, 0.0, sin( rottrigger ) ) );
                model2World.setScale( QVector3D(0.5, 0.5, 0.5) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 3 );

                trianglePoly.array.release();

                linePoly.array.bind();

                rottrigger *= 2.0;
                float c = cos( rottrigger );
                float s = sin( rottrigger );

                model2World.setTranslation( QVector3D( xtrigger - s * halfWidthTrigger,
                                                       ytrigger + c * halfWidthTrigger,
                                                       0.55 ) );

                model2World.setScale( QVector3D(halfWidthTrigger * 2.0,1.0,1.0) );

                float angle = atan2( -c, s );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(4.0);
                glDrawArrays(GL_LINES, 0, 2 );

                linePoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SVHCL[%d]SLOT[%d] POS/TRIG",scnrEdit->sVehicle[i]->ID,j);

                    model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                           ytrigger + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }
            }

            if( scnrEdit->sVehicle[i]->sItem[j]->cond.TTCTrigger == true && scnrEdit->sVehicle[i]->sItem[j]->cond.ttcCalType == 0 ){

                float xtrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ttcCalPosX;
                float ytrigger = scnrEdit->sVehicle[i]->sItem[j]->cond.ttcCalPosY;

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( xtrigger, ytrigger, 0.55) );
                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(0.50,0.50,0.50) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SVHCL[%d]SLOT[%d] TTC/TRIG/POINT",scnrEdit->sVehicle[i]->ID,j);

                    model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                           ytrigger + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }
            }

            if( scnrEdit->sVehicle[i]->sItem[j]->act.actionType == 0 &&
                    scnrEdit->sVehicle[i]->sItem[j]->act.fParams.size() >= 3 &&
                    scnrEdit->sVehicle[i]->sItem[j]->act.iParams.size() >= 2 ){

                float xat = scnrEdit->sVehicle[i]->sItem[j]->act.fParams[0];
                float yat = scnrEdit->sVehicle[i]->sItem[j]->act.fParams[1];
                float rot = scnrEdit->sVehicle[i]->sItem[j]->act.fParams[2] * 0.017452 * 0.5;

                int modelID = scnrEdit->sVehicle[i]->sItem[j]->act.iParams[0];
                float hl = scnrEdit->setDlg->GetVehicleKindTableStr( modelID, 2 ).toFloat() * 0.5;
                float hw = scnrEdit->setDlg->GetVehicleKindTableStr( modelID, 3 ).toFloat() * 0.5;

                vhclPoly.array.bind();

                model2World.setTranslation( QVector3D( xat, yat, 0.55) );
                model2World.setRotation( QQuaternion( cos(rot), 0.0, 0.0, sin(rot) ) );
                model2World.setScale( QVector3D(hl,hw,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

                glLineWidth(1.0);
//                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4 * 6 );

                vhclPoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SVHCL[%d] APPEAR",scnrEdit->sVehicle[i]->ID);

                    model2World.setTranslation( QVector3D( xat + hl + 0.5,
                                                           yat + hw + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }


                // Draw Path Route
                if( scnrEdit->sVehicle[i]->sItem[j]->act.iParams[2] == 1 &&
                        scnrEdit->sVehicle[i]->sItem[j]->act.laneShape.size() > 0 ){

                    bool sVselected = scnrEdit->isScenarioVehicleSelected( scnrEdit->sVehicle[i]->ID );

                    for(int k=0;k<scnrEdit->sVehicle[i]->sItem[j]->act.laneShape.size();k++){

                        linePoly.array.bind();

                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_useTex, 2 );
                        glLineWidth( laneDrawWidth );
                        if( sVselected == true ){
                            program->setUniformValue( u_vColor, QVector4D( 1.0, 0.80, 0.74, 1.0 ) );
                        }
                        else{
                            program->setUniformValue( u_vColor, QVector4D( 0.6, 0.8, 1.0, 1.0 ) );
                        }

                        struct LaneShapeInfo *s = scnrEdit->sVehicle[i]->sItem[j]->act.laneShape[k];

                        for(int l=0;l<s->pos.size()-1;++l){

                            model2World.setTranslation( QVector3D( s->pos[l]->x(),
                                                                   s->pos[l]->y(),
                                                                   s->pos[l]->z()) );

                            model2World.setScale( QVector3D(s->segmentLength[l],1.0,1.0) );

                            float pitch = asin( (s->pos[l]->z() - s->pos[l+1]->z()) / s->segmentLength[l]) ;

                            float angle = s->angles[l];
                            model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) * QQuaternion( cos(pitch*0.5), 0.0 , sin(pitch*0.5) , 0.0 ) );

                            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                            glDrawArrays(GL_LINES, 0, 2 );
                        }

                        linePoly.array.release();

                        if(k == 0){
                            float xw = s->pos.first()->x();
                            float yw = s->pos.first()->y();
                            float zw = s->pos.first()->z();

                            circlePoly.array.bind();

                            model2World.setTranslation( QVector3D( xw, yw, zw + 0.05) );
                            model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                            model2World.setScale( QVector3D(0.30,0.30,0.30) );

                            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                            program->setUniformValue( u_isText, 0 );
                            program->setUniformValue( u_useTex, 2 );
                            program->setUniformValue( u_vColor, QVector4D( 0.2, 0.5, 1.0, 1.0 ) );

                            glLineWidth(1.0);
                            glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                            circlePoly.array.release();
                        }


                        float xw = s->pos.last()->x();
                        float yw = s->pos.last()->y();
                        float zw = s->pos.last()->z();

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( xw, yw, zw + 0.05) );
                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                        model2World.setScale( QVector3D(0.30,0.30,0.30) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_vColor, QVector4D( 0.2, 0.5, 1.0, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }
                }

            }

        }
    }

    for(int i=0;i<scnrEdit->sPedest.size();++i){

        for(int j=0;j<scnrEdit->sPedest[i]->sItem.size();++j){

            if( scnrEdit->sPedest[i]->sItem[j]->cond.positionTrigger == true ){

                float xtrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ptX;
                float ytrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ptY;
                float rottrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ptPassAngle * 0.017452 * 0.5;
                float halfWidthTrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ptWidth * 0.5;


                trianglePoly.array.bind();

                model2World.setTranslation( QVector3D( xtrigger,ytrigger,0.55) );
                model2World.setRotation( QQuaternion( cos( rottrigger ), 0.0, 0.0, sin( rottrigger ) ) );
                model2World.setScale( QVector3D(0.5, 0.5, 0.5) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 3 );

                trianglePoly.array.release();

                linePoly.array.bind();

                rottrigger *= 2.0;
                float c = cos( rottrigger );
                float s = sin( rottrigger );

                model2World.setTranslation( QVector3D( xtrigger - s * halfWidthTrigger,
                                                       ytrigger + c * halfWidthTrigger,
                                                       0.55 ) );

                model2World.setScale( QVector3D(halfWidthTrigger * 2.0,1.0,1.0) );

                float angle = atan2( -c, s );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(4.0);
                glDrawArrays(GL_LINES, 0, 2 );

                linePoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SPED[%d]SLOT[%d] POS/TRIG",scnrEdit->sPedest[i]->ID,j);

                    model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                           ytrigger + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }
            }

            if( scnrEdit->sPedest[i]->sItem[j]->cond.TTCTrigger == true && scnrEdit->sPedest[i]->sItem[j]->cond.ttcCalType == 0 ){

                float xtrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ttcCalPosX;
                float ytrigger = scnrEdit->sPedest[i]->sItem[j]->cond.ttcCalPosY;

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( xtrigger, ytrigger, 0.55) );
                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(0.50,0.50,0.50) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SPED[%d]SLOT[%d] TTC/TRIG/POINT",scnrEdit->sPedest[i]->ID,j);

                    model2World.setTranslation( QVector3D( xtrigger + 0.5,
                                                           ytrigger + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }
            }

            if( scnrEdit->sPedest[i]->sItem[j]->act.actionType == 0 &&
                    scnrEdit->sPedest[i]->sItem[j]->act.fParams.size() >= 3 &&
                    scnrEdit->sPedest[i]->sItem[j]->act.iParams.size() >= 1 ){

                float xat = scnrEdit->sPedest[i]->sItem[j]->act.fParams[0];
                float yat = scnrEdit->sPedest[i]->sItem[j]->act.fParams[1];
                float rot = scnrEdit->sPedest[i]->sItem[j]->act.fParams[2] * 0.017452 * 0.5;

                int modelID = scnrEdit->sPedest[i]->sItem[j]->act.iParams[0];
                float hl = scnrEdit->setDlg->GetPedestKindTableStr( modelID, 2 ).toFloat() * 0.5;
                float hw = scnrEdit->setDlg->GetPedestKindTableStr( modelID, 3 ).toFloat() * 0.5;

                vhclPoly.array.bind();

                model2World.setTranslation( QVector3D( xat, yat, 0.55) );
                model2World.setRotation( QQuaternion( cos(rot), 0.0, 0.0, sin(rot) ) );
                model2World.setScale( QVector3D(hl,hw,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

                glLineWidth(1.0);
                //glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4 * 6 );

                vhclPoly.array.release();

                if( textPoly.isTextValid == true ){

                    textPoly.textArray.bind();
                    textPoly.textBuffer->bind();

                    program->setUniformValue( u_useTex, 100 );
                    program->setUniformValue( u_isText, 100 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                    char str[50];
                    sprintf(str,"SPEDEST[%d] APPEAR",scnrEdit->sPedest[i]->ID);

                    model2World.setTranslation( QVector3D( xat + hl + 0.5,
                                                           yat + hw + 0.5,
                                                           0.60) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }

                    textPoly.textBuffer->release();
                    textPoly.textArray.release();
                }

                // Draw PedestLane Route
                if( scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape.size() > 0 ){

                    bool sPselected = scnrEdit->isScenarioPedestrianSelected( scnrEdit->sPedest[i]->ID );

                    for(int k=0;k<scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape.size()-1;k++){

                        linePoly.array.bind();

                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_useTex, 2 );
                        glLineWidth( laneDrawWidth );
                        if( sPselected == true ){
                            program->setUniformValue( u_vColor, QVector4D( 1.0, 0.80, 0.74, 1.0 ) );
                        }
                        else{
                            program->setUniformValue( u_vColor, QVector4D( 0.6, 0.8, 1.0, 1.0 ) );
                        }

                        model2World.setTranslation( scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape[k]->pos );

                        float len = scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape[k]->distanceToNextPos;
                        model2World.setScale( QVector3D(len,1.0,1.0) );

                        float angle = scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape[k]->angleToNextPos;
                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                        linePoly.array.release();
                    }

                    if( circlePoly.isValid == true ){

                        circlePoly.array.bind();

                        model2World.setTranslation( scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape[0]->pos );

                        model2World.setRotation( QQuaternion( 1.0, 0.0 , 0.0 , 0.0 )  );

                        float w = scnrEdit->sPedest[i]->sItem[j]->act.pedestLaneshape[0]->width * 0.5;
                        model2World.setScale( QVector3D(w,w,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }
                }
            }

        }
    }


    //
    // Lane
    if( linePoly.isValid == true && road != NULL && showLanesFlag == true ){

        for(int i=0;i<road->lanes.size();++i){

            bool isSelected = false;
            for(int j=0;j<selectedObj.selObjKind.size();++j){
                if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_LANE ){
                    if( selectedObj.selObjID[j] == road->lanes[i]->id ){
                        isSelected = true;
                        break;
                    }
                }
                if( LaneListFlag == true  && j == 0 &&
                        selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE && nodePickModeFlag == false ){

                    int ndIdx = road->indexOfNode( selectedObj.selObjID[j] );
                    if( ndIdx >= 0 ){
                        if( road->nodes[ndIdx]->laneList.size() > 0 ){

                            int llIdx = 0;
                            int numList = 0;
                            for(int k=0;k<road->nodes[ndIdx]->laneList.size();++k){
                                numList += road->nodes[ndIdx]->laneList[k]->lanes.size();
                                if( laneListIndex < numList ){
                                    llIdx = k;
                                    numList -= road->nodes[ndIdx]->laneList[k]->lanes.size();
                                    break;
                                }
                            }

                            if( road->nodes[ndIdx]->laneList[llIdx]->lanes.size() > laneListIndex - numList ){
                                if( road->nodes[ndIdx]->laneList[llIdx]->lanes[laneListIndex - numList].contains(road->lanes[i]->id) == true ){
                                    isSelected = true;
                                }
                            }
                        }
                    }
                }
                if( RouteLaneListFlag == true && j == 0 && selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE && nodePickModeFlag == false ){

                    int ndIdx = road->indexOfNode( selectedObj.selObjID[j] );
                    if( ndIdx >= 0 && road->nodes[ndIdx]->isOriginNode == true && road->nodes[ndIdx]->odData.size() > 0 ){

                        int destNode = odRoute->GetCurrentDestinationNode();
                        for(int n=0;n<road->nodes[ndIdx]->odData.size();++n){
                            if( road->nodes[ndIdx]->odData[n]->destinationNode != destNode ){
                                continue;
                            }
                            for(int m=0;m<road->nodes[ndIdx]->odData[n]->route.size();++m){
                                for(int k=0;k<road->nodes[ndIdx]->odData[n]->route[m]->routeLaneLists.size();++k){
                                    for(int l=0;l<road->nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[k]->laneList.size();++l){
                                        if( road->nodes[ndIdx]->odData[n]->route[m]->routeLaneLists[k]->laneList[l].indexOf(road->lanes[i]->id) >= 0 ){
                                            isSelected = true;
                                            break;
                                        }
                                    }
                                    if( isSelected == true ){
                                        break;
                                    }
                                }
                                if( isSelected == true ){
                                    break;
                                }
                            }
                            if( isSelected == true ){
                                break;
                            }
                        }
                    }
                }
            }

            bool isEdgeStartSelected = false;
            bool isEdgeEndSelected = false;
            for(int j=0;j<selectedObj.selObjKind.size();++j){
                if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_LANE_EDGE_START ){
                    if( selectedObj.selObjID[j] == road->lanes[i]->id ){
                        isEdgeStartSelected = true;
                    }
                }
                else if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_LANE_EDGE_END ){
                    if( selectedObj.selObjID[j] == road->lanes[i]->id ){
                        isEdgeEndSelected = true;
                    }
                }
            }

            bool isRelatedLane = false;
            if( RelatedLanesFlag == true ){
                if( selectedObj.selObjKind.size() > 0){
                    if( selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE ){
                        int nIdx = road->indexOfNode( selectedObj.selObjID[0] );
                        if( nIdx >= 0 ){
                            if( road->nodes[nIdx]->relatedLanes.indexOf( road->lanes[i]->id ) >= 0 ){
                                isRelatedLane = true;
                            }
                        }
                    }
                }
            }

            bool isScenarioNodeRouteLane = false;
            for(int j=0;j<scnrEdit->sVehicle.size();++j){
                if( scnrEdit->isScenarioVehicleSelected( scnrEdit->sVehicle[j]->ID ) == true ){
                    for(int k=0;k<scnrEdit->sVehicle[j]->sItem.size();++k){
                        if( scnrEdit->sVehicle[j]->sItem[k]->act.actionType == 0 &&
                                scnrEdit->sVehicle[j]->sItem[k]->act.iParams.size() >= 3 &&
                                scnrEdit->sVehicle[j]->sItem[k]->act.iParams[2] == 0 &&
                                scnrEdit->sVehicle[j]->sItem[k]->act.route != NULL ){

                            for(int l=0;l<scnrEdit->sVehicle[j]->sItem[k]->act.route->routeLaneLists.size();++l){
                                for(int m=0;m<scnrEdit->sVehicle[j]->sItem[k]->act.route->routeLaneLists[l]->laneList.size();++m){
                                    if( scnrEdit->sVehicle[j]->sItem[k]->act.route->routeLaneLists[l]->laneList[m].contains( road->lanes[i]->id ) == true ){
                                        isScenarioNodeRouteLane = true;
                                        break;
                                    }
                                }
                                if( isScenarioNodeRouteLane == true ){
                                    break;
                                }
                            }
                            if( isScenarioNodeRouteLane == true ){
                                break;
                            }
                        }
                    }
                }
                if( isScenarioNodeRouteLane == true ){
                    break;
                }
            }

            linePoly.array.bind();


            program->setUniformValue( u_isText, 0 );


            if( isSelected == true || isScenarioNodeRouteLane == true ){
                program->setUniformValue( u_useTex, 2 );
                glLineWidth( laneDrawWidth * 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.5, 0.5, 1.0 ) );
            }
            else if( road->lanes[i]->suspectError == true ){
                program->setUniformValue( u_useTex, 2 );
                glLineWidth( laneDrawWidth * 3 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.0, 1.0 ) );
            }
            else{

                if( isRelatedLane == true ){
                    program->setUniformValue( u_useTex, 2 );
                    glLineWidth( laneDrawWidth );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.92, 0.66, 1.0 ) );
                }
                else{

                    if( colorLaneByODDFlag == true && road->lanes[i]->automaticDrivingEnabled == true ){

                        float r = 0.7;
                        float g = 0.2;
                        float b = 0.1;

                        program->setUniformValue( u_useTex, 2 );
                        glLineWidth( laneDrawWidth );
                        program->setUniformValue( u_vColor, QVector4D( r, g, b, 1.0 ) );
                    }
                    else if( colorLaneByActualSpeedFlag == true ){

                        float r = 1.0;
                        float g = 1.0;
                        float b = 1.0;

                        float h = (road->lanes[i]->actualSpeed / 150.0) * 6.0;
                        if( h > 6.0 ){
                            h = 6.0;
                        }
                        int cIdx = (int)h;
                        float f = h - (float)cIdx;
                        switch ( cIdx ) {
                        default:
                        case 0: g *= f;       b = 0.0;      break;
                        case 1: r *= 1.0 - f; b = 0.0;      break;
                        case 2: r = 0.0;      b *= f;       break;
                        case 3: r = 0.0;      g *= 1.0 - f; break;
                        case 4: r *= f;       g = 0.0;      break;
                        case 5: g = 0.0;      b *= 1.0 - f; break;
                        }

                        program->setUniformValue( u_useTex, 2 );
                        glLineWidth( laneDrawWidth );
                        program->setUniformValue( u_vColor, QVector4D( r, g, b, 1.0 ) );
                    }
                    else if( colorLaneBySpeedLimitFlag == true ){

                        float r = 1.0;
                        float g = 1.0;
                        float b = 1.0;

                        float h = (road->lanes[i]->speedInfo / 150.0) * 6.0;
                        if( h > 6.0 ){
                            h = 6.0;
                        }
                        int cIdx = (int)h;
                        float f = h - (float)cIdx;
                        switch ( cIdx ) {
                        default:
                        case 0: g *= f;       b = 0.0;      break;
                        case 1: r *= 1.0 - f; b = 0.0;      break;
                        case 2: r = 0.0;      b *= f;       break;
                        case 3: r = 0.0;      g *= 1.0 - f; break;
                        case 4: r *= f;       g = 0.0;      break;
                        case 5: g = 0.0;      b *= 1.0 - f; break;
                        }

                        program->setUniformValue( u_useTex, 2 );
                        glLineWidth( laneDrawWidth );
                        program->setUniformValue( u_vColor, QVector4D( r, g, b, 1.0 ) );
                    }
                    else{
                        program->setUniformValue( u_useTex, 0 );
                        glLineWidth( laneDrawWidth );
                        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 0.0, 0.0 ) );
                    }

                }

            }

            for(int j=0;j<road->lanes[i]->shape.pos.size() - 1;++j){

                model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos[j]->x(),
                                                       road->lanes[i]->shape.pos[j]->y(),
                                                       road->lanes[i]->shape.pos[j]->z()) );

                model2World.setScale( QVector3D(road->lanes[i]->shape.segmentLength[j],1.0,1.0) );

                float pitch = asin( (road->lanes[i]->shape.pos[j]->z() - road->lanes[i]->shape.pos[j+1]->z()) / road->lanes[i]->shape.segmentLength[j]) ;

                float angle = road->lanes[i]->shape.angles[j];
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) * QQuaternion( cos(pitch*0.5), 0.0 , sin(pitch*0.5) , 0.0 ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glDrawArrays(GL_LINES, 0, 2 );
            }

            glLineWidth(3.0);

            if( road->lanes[i]->previousLanes.size() == 0 ){

                float c = road->lanes[i]->shape.derivative.first()->x();
                float s = road->lanes[i]->shape.derivative.first()->y();

                float wd = road->lanes[i]->laneWidth * 0.5;
                model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.first()->x() - s * wd,
                                                       road->lanes[i]->shape.pos.first()->y() + c * wd,
                                                       road->lanes[i]->shape.pos.first()->z()) );

                model2World.setScale( QVector3D( road->lanes[i]->laneWidth ,1.0,1.0) );

                float angle = atan2( -c, s );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                program->setUniformValue( u_vColor, QVector4D( 0.5, 0.3, 0.3, 1.0 ) );

                glDrawArrays(GL_LINES, 0, 2 );
            }

            {
                float c = road->lanes[i]->shape.derivative.last()->x();
                float s = road->lanes[i]->shape.derivative.last()->y();

                float wd = road->lanes[i]->laneWidth * 0.5;
                model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.last()->x() - s * wd,
                                                       road->lanes[i]->shape.pos.last()->y() + c * wd,
                                                       road->lanes[i]->shape.pos.last()->z()) );

                model2World.setScale( QVector3D( road->lanes[i]->laneWidth ,1.0,1.0) );

                float angle = atan2( -c, s );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                program->setUniformValue( u_vColor, QVector4D( 0.5, 0.3, 0.3, 1.0 ) );

                glDrawArrays(GL_LINES, 0, 2 );
            }

            linePoly.array.release();
            glLineWidth(1.0);


            if( isEdgeEndSelected == true || isEdgeStartSelected == true ){

                circlePoly.array.bind();

                if( isEdgeEndSelected == true ){
                    model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.last()->x(),
                                                           road->lanes[i]->shape.pos.last()->y(),
                                                           0.1) );
                }
                else if( isEdgeStartSelected == true ){
                    model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.first()->x(),
                                                           road->lanes[i]->shape.pos.first()->y(),
                                                           0.1) );
                }

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(0.4,0.4,0.4) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_useTex, 0 );
                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 0.0, 0.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();
            }



            // StopPoint
            {
                for(int j=0;j<road->lanes[i]->stopPoints.size();++j){

                    float xsp = road->lanes[i]->stopPoints[j]->pos.x();
                    float ysp = road->lanes[i]->stopPoints[j]->pos.y();

                    if( circlePoly.isValid == true ){

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( xsp, ysp, 0.1) );
                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                        model2World.setScale( QVector3D(0.15,0.15,0.15) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.2, 1.0, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }
                }
            }


            // CrossPoint
            {
                for(int j=0;j<road->lanes[i]->crossPoints.size();++j){

                    float xsp = road->lanes[i]->crossPoints[j]->pos.x();
                    float ysp = road->lanes[i]->crossPoints[j]->pos.y();

                    if( circlePoly.isValid == true ){

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( xsp, ysp, 0.1) );
                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );

                        if( road->lanes[i]->crossPoints[j]->duality == false ){
                            program->setUniformValue( u_vColor, QVector4D( 1.0, 0.85, 0.1, 1.0 ) );
                            model2World.setScale( QVector3D(0.25,0.25,0.15) );
                        }
                        else{
                            program->setUniformValue( u_vColor, QVector4D( 0.25, 0.0, 1.0, 1.0 ) );
                            model2World.setScale( QVector3D(0.15,0.15,0.15) );
                        }

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );


                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }
                }
            }

            // PedestCrossPoint
            {
                for(int j=0;j<road->lanes[i]->pedestCrossPoints.size();++j){

                    float xsp = road->lanes[i]->pedestCrossPoints[j]->pos.x();
                    float ysp = road->lanes[i]->pedestCrossPoints[j]->pos.y();

                    if( circlePoly.isValid == true ){

                        circlePoly.array.bind();

                        model2World.setTranslation( QVector3D( xsp, ysp, 0.1) );
                        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                        model2World.setScale( QVector3D(0.15,0.15,0.15) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_vColor, QVector4D( 0.25, 1.0, 0.25, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }
                }
            }

            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                char str[25];
                sprintf(str,"LANE#%d",road->lanes[i]->id);

                int mid = road->lanes[i]->shape.pos.size() / 2;
                if( mid > 0 ){
                    model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos[mid]->x(),
                                                           road->lanes[i]->shape.pos[mid]->y(),
                                                           road->lanes[i]->shape.pos[mid]->z()) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showLaneLabelsFlag == true && showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }
        }
    }

    model2World.setScale( QVector3D(1.0, 1.0, 1.0) );


    //
    // Node
    if( road != NULL && showNodesFlag == true ){

        for(int i=0;i<road->nodes.size();++i){

            bool isSelected = false;
            for(int j=0;j<selectedObj.selObjKind.size();++j){
                if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE ){
                    if( selectedObj.selObjID[j] == road->nodes[i]->id ){
                        isSelected = true;
                        break;
                    }
                }
                else if( selectedObj.selObjKind[j] == _SEL_OBJ::SEL_NODE_ROUTE_PICK ){
                    if( selectedObj.selObjID[j] == road->nodes[i]->id ){
                        isSelected = true;
                        break;
                    }
                }
            }

            bool isDestNode = false;
            if( selectedObj.selObjKind.size() > 0 && (selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE || selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE_ROUTE_PICK) ){

                int ndIdx = road->indexOfNode( selectedObj.selObjID[0] );
                if( ndIdx >= 0 && road->nodes[ndIdx]->isOriginNode == true && road->nodes[ndIdx]->odData.size() > 0 ){

                    int destNode = odRoute->GetCurrentDestinationNode();
                    if( destNode == road->nodes[i]->id ){
                        isDestNode = true;
                    }
                }
            }

            if( circlePoly.isValid == true ){

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( road->nodes[i]->pos.x(),
                                                       road->nodes[i]->pos.y(),
                                                       0.0) );

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(5.0, 5.0, 1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_useTex, 0 );
                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 0.0, 0.0 ) );

                if( isSelected == true ){
                    glLineWidth(8.0);
                }
                else if( road->nodes[i]->suspectError == true ){
                    glLineWidth(15.0);
                    program->setUniformValue( u_useTex, 2 );
                    program->setUniformValue( u_isText, 0 );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.0, 1.0 ) );
                }
                else{
                    glLineWidth(3.0);
                }

                if( isDestNode == true ){
                    glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );
                }
                else{
                    glDrawArrays(GL_LINE_LOOP, 0, NODE_CIRCLE_DIV );
                }

                glLineWidth(1.0);

                circlePoly.array.release();
            }

            if( road->nodes[i]->isOriginNode == true && circlePoly.isValid == true ){

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( road->nodes[i]->pos.x(),
                                                       road->nodes[i]->pos.y(),
                                                       0.0) );

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(7.5, 7.5, 7.5) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.0, 1.0 ) );

                glLineWidth(6.0);

                glDrawArrays(GL_LINE_LOOP, 0, NODE_CIRCLE_DIV );
                glLineWidth(1.0);

                circlePoly.array.release();
            }

            if( road->nodes[i]->isDestinationNode == true && circlePoly.isValid == true ){

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( road->nodes[i]->pos.x(),
                                                       road->nodes[i]->pos.y(),
                                                       0.0) );

                model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );
                model2World.setScale( QVector3D(10.0, 10.0, 10.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );


                glLineWidth(6.0);

                glDrawArrays(GL_LINE_LOOP, 0, NODE_CIRCLE_DIV );
                glLineWidth(1.0);

                circlePoly.array.release();
            }

            if( linePoly.isValid == true ){

                linePoly.array.bind();

                model2World.setTranslation( QVector3D( road->nodes[i]->pos.x(),
                                                       road->nodes[i]->pos.y(),
                                                       0.0) );

                model2World.setScale( QVector3D(7.0,7.0,1.0) );

                for(int j=0;j<road->nodes[i]->legInfo.size();++j){

                    float angle = road->nodes[i]->legInfo[j]->angle * 0.017452;
                    //qDebug() << "angle = " << angle;

                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                    program->setUniformValue( u_isText, 0 );

                    if( isSelected == true && numberKeyPressed > 0 && numberKeyPressed == j + 1 ){
                        glLineWidth(6.0);

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_vColor, QVector4D( 0.8, 0.1, 0.1, 1.0 ) );
                    }
                    else{
                        glLineWidth(1.0);
                        program->setUniformValue( u_useTex, 0 );
                    }

                    glDrawArrays(GL_LINES, 0, 2 );
                    glLineWidth(1.0);
                }

                linePoly.array.release();
            }


            // Traffic Signal
            if( showTrafficSignalsFlag == true ){

                for(int j=0;j<road->nodes[i]->trafficSignals.size();++j){

                    bool isTSSelected = false;
                    for(int k=0;k<selectedObj.selObjKind.size();++k){
                        if( selectedObj.selObjKind[k] == _SEL_OBJ::SEL_TRAFFIC_SIGNAL ){
                            if( selectedObj.selObjID[k] == road->nodes[i]->trafficSignals[j]->id ){
                                isTSSelected = true;
                                break;
                            }
                        }
                    }

                    int type = road->nodes[i]->trafficSignals[j]->type;

                    float angle = road->GetNodeLegAngle( road->nodes[i]->trafficSignals[j]->relatedNode,
                                                         road->nodes[i]->trafficSignals[j]->controlNodeDirection );
                    angle *= 0.017452;

                    float cp = cos( angle );
                    float sp = sin( angle );

                    if( type == 0 ){  // for vehicles

                        float shift = -2.0;
                        if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
                            shift = 2.0;
                        }
                        float x_TS = road->nodes[i]->trafficSignals[j]->pos.x();
                        float y_TS = road->nodes[i]->trafficSignals[j]->pos.y();

                        for(int k=0;k<3;++k){
                            float LensSize = 1.0;
                            if( isTSSelected == true ){
                                LensSize = 2.0;
                            }
                            float xL = sp * LensSize * (1 - k);
                            float yL = cp * LensSize * (1 - k) * (-1.0);

                            if( circlePoly.isValid == true ){

                                circlePoly.array.bind();

                                model2World.setTranslation( QVector3D( x_TS + xL,
                                                                       y_TS + yL,
                                                                       2.0) );

                                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) )  );

                                LensSize /= 2;
                                model2World.setScale( QVector3D(LensSize,LensSize,LensSize) );

                                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                                program->setUniformValue( u_useTex, 2 );
                                program->setUniformValue( u_isText, 0 );
                                if( k == 0 ){
                                    program->setUniformValue( u_vColor, QVector4D( 0.0, 0.7137, 0.9529, 1.0 ) );
                                }
                                if( k == 1 ){
                                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.945, 0.0, 1.0 ) );
                                }
                                if( k == 2 ){
                                    program->setUniformValue( u_vColor, QVector4D( 0.737, 0.188, 0.1724, 1.0 ) );
                                }

                                glLineWidth(1.0);
                                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );
                                glLineWidth(1.0);

                                circlePoly.array.release();
                            }

                        }

                    }
                    else if( type == 1 ){  // for pedestrian

                        float shift = -3.0;
                        if( road->LeftOrRight == RIGHT_HAND_TRAFFIC ){
                            shift = 3.0;
                        }
                        float x_TS = road->nodes[i]->trafficSignals[j]->pos.x();
                        float y_TS = road->nodes[i]->trafficSignals[j]->pos.y();

                        for(int k=0;k<2;++k){
                            float LensSize = 0.7;
                            if( isTSSelected == true ){
                                LensSize = 1.4;
                            }
                            float xL = cp * LensSize * (1 - k);
                            float yL = sp * LensSize * (1 - k);

                            if( circlePoly.isValid == true ){
                                circlePoly.array.bind();

                                model2World.setTranslation( QVector3D( x_TS + xL,
                                                                       y_TS + yL,
                                                                       2.0) );

                                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) )  );
                                LensSize /= 2;
                                model2World.setScale( QVector3D(LensSize,LensSize,LensSize) );

                                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                                program->setUniformValue( u_useTex, 2 );
                                program->setUniformValue( u_isText, 0 );
                                if( k == 0 ){
                                    program->setUniformValue( u_vColor, QVector4D( 0.0, 0.7137, 0.9529, 1.0 ) );
                                }
                                if( k == 1 ){
                                    program->setUniformValue( u_vColor, QVector4D( 0.737, 0.188, 0.1724, 1.0 ) );
                                }

                                glLineWidth(1.0);
                                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );
                                glLineWidth(1.0);

                                circlePoly.array.release();
                            }
                        }
                    }
                }
            }

            // StopLine
            if( showStopLinesFlag == true ){

                for(int j=0;j<road->nodes[i]->stopLines.size();++j){

                    bool isSLSelected = false;
                    for(int k=0;k<selectedObj.selObjKind.size();++k){
                        if( selectedObj.selObjKind[k] == _SEL_OBJ::SEL_STOPLINE ){
                            if( selectedObj.selObjID[k] == road->nodes[i]->stopLines[j]->id ){
                                isSLSelected = true;
                                break;
                            }
                        }
                    }

                    if( linePoly.isValid == true ){

                        linePoly.array.bind();

                        model2World.setTranslation( QVector3D( road->nodes[i]->stopLines[j]->leftEdge.x(),
                                                               road->nodes[i]->stopLines[j]->leftEdge.y(),
                                                               -0.15) );

                        float dx = road->nodes[i]->stopLines[j]->rightEdge.x() - road->nodes[i]->stopLines[j]->leftEdge.x();
                        float dy = road->nodes[i]->stopLines[j]->rightEdge.y() - road->nodes[i]->stopLines[j]->leftEdge.y();
                        float L = sqrt( dx * dx + dy * dy );
                        float angle = atan2( dy, dx );

                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                        model2World.setScale( QVector3D(L,1.0,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 0 );
                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_vColor, QVector4D( 1.0, 0.9, 0.8, 1.0 ) );

                        glLineWidth(8.0);
                        if( isSLSelected == true ){
                            glLineWidth(15.0);
                        }
                        glDrawArrays(GL_LINES, 0, 2 );
                        glLineWidth(1.0);

                        linePoly.array.release();
                    }
                }
            }


            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();
                textPoly.textBuffer->bind();

                program->setUniformValue( u_useTex, 100 );
                program->setUniformValue( u_isText, 100 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                // Node ID
                {
                    char str[25];
                    sprintf(str,"NODE#%d",road->nodes[i]->id);

                    model2World.setTranslation( QVector3D( road->nodes[i]->pos.x(),
                                                           road->nodes[i]->pos.y(),
                                                           0.0) );

                    QQuaternion letterQuat = cameraQuat.conjugated();
                    model2World.setRotation( letterQuat );
                    model2World.setScale( QVector3D(1.0,1.0,1.0) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glActiveTexture( GL_TEXTURE0 );

                    float x = 0.0;
                    float y = 0.0;
                    float scale = FONT_SCALE;

                    if( showNodeLabelsFlag == true && showLabelsFlag == true ){
                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }
                }

                // Leg ID
                {
                    for(int j=0;j<road->nodes[i]->legInfo.size();++j){

                        char str[25];
                        sprintf(str,"[ %d ]",road->nodes[i]->legInfo[j]->legID);

                        float angle = road->nodes[i]->legInfo[j]->angle * 0.017452;
                        float xAt = road->nodes[i]->pos.x() + cos(angle) * 7.4;
                        float yAt = road->nodes[i]->pos.y() + sin(angle) * 7.4;
                        model2World.setTranslation( QVector3D( xAt, yAt, 0.0) );

                        QQuaternion letterQuat = cameraQuat.conjugated();
                        model2World.setRotation( letterQuat );
                        model2World.setScale( QVector3D(1.0,1.0,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glActiveTexture( GL_TEXTURE0 );

                        float x = 0.0;
                        float y = 0.0;
                        float scale = FONT_SCALE;

                        if( showNodeLabelsFlag == true && showLabelsFlag == true ){
                            for(unsigned int c=0;c<strlen(str);++c ){

                                Character* ch = Characters[ str[c] ];

                                GLfloat xpos = x + ch->Bearing.width() * scale;
                                GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                                float w = ch->Size.width() * scale;
                                float h = ch->Size.height() * scale;

                                QVector<GLfloat> fontPoly;
                                fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                                fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                                textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                                x += ( ch->Advance >> 6 ) * scale;
                            }
                        }
                    }
                }

                // TS ID
                {
                    for(int j=0;j<road->nodes[i]->trafficSignals.size();++j){

                        char str[25];
                        if( road->nodes[i]->trafficSignals[j]->type == 0 ){
                            sprintf(str,"VTS#%d",road->nodes[i]->trafficSignals[j]->id);
                        }
                        else if( road->nodes[i]->trafficSignals[j]->type == 1 ){
                            sprintf(str,"PTS#%d",road->nodes[i]->trafficSignals[j]->id);
                        }

                        float xAt = road->nodes[i]->trafficSignals[j]->pos.x();
                        float yAt = road->nodes[i]->trafficSignals[j]->pos.y();
                        model2World.setTranslation( QVector3D( xAt, yAt, 0.0) );

                        QQuaternion letterQuat = cameraQuat.conjugated();
                        model2World.setRotation( letterQuat );
                        model2World.setScale( QVector3D(1.0,1.0,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glActiveTexture( GL_TEXTURE0 );

                        float x = 0.0;
                        float y = 0.0;
                        float scale = FONT_SCALE;

                        if( showTrafficSignalLabelsFlag == true && showLabelsFlag == true ){
                            for(unsigned int c=0;c<strlen(str);++c ){

                                Character* ch = Characters[ str[c] ];

                                GLfloat xpos = x + ch->Bearing.width() * scale;
                                GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                                float w = ch->Size.width() * scale;
                                float h = ch->Size.height() * scale;

                                QVector<GLfloat> fontPoly;
                                fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                                fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                                textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                                x += ( ch->Advance >> 6 ) * scale;
                            }
                        }
                    }
                }

                // Stopline ID
                {
                    for(int j=0;j<road->nodes[i]->stopLines.size();++j){

                        char str[25];
                        char typeChar[] = { 'S', 'Y', 'T', 'C'  };
                        sprintf(str,"SL#%d[%c]",road->nodes[i]->stopLines[j]->id, typeChar[ road->nodes[i]->stopLines[j]->stopLineType ]);

                        if( road->LeftOrRight == LEFT_HAND_TRAFFIC ){
                            float xAt = road->nodes[i]->stopLines[j]->leftEdge.x();
                            float yAt = road->nodes[i]->stopLines[j]->leftEdge.y();
                            model2World.setTranslation( QVector3D( xAt, yAt, 0.0) );
                        }
                        else{
                            float xAt = road->nodes[i]->stopLines[j]->rightEdge.x();
                            float yAt = road->nodes[i]->stopLines[j]->rightEdge.y();
                            model2World.setTranslation( QVector3D( xAt, yAt, 0.0) );
                        }


                        QQuaternion letterQuat = cameraQuat.conjugated();
                        model2World.setRotation( letterQuat );
                        model2World.setScale( QVector3D(1.0,1.0,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glActiveTexture( GL_TEXTURE0 );

                        float x = 0.0;
                        float y = 0.0;
                        float scale = FONT_SCALE;

                        if( showTrafficSignalLabelsFlag == true && showLabelsFlag == true ){
                            for(unsigned int c=0;c<strlen(str);++c ){

                                Character* ch = Characters[ str[c] ];

                                GLfloat xpos = x + ch->Bearing.width() * scale;
                                GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                                float w = ch->Size.width() * scale;
                                float h = ch->Size.height() * scale;

                                QVector<GLfloat> fontPoly;
                                fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                                fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                                fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                                textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                                x += ( ch->Advance >> 6 ) * scale;
                            }
                        }
                    }
                }

                textPoly.textBuffer->release();
                textPoly.textArray.release();
            }
        }

        if( nodePickModeFlag == true && linePoly.isValid == true ){

            for(int i=1;i<selectedObj.selObjID.size();++i){

                linePoly.array.bind();

                int nd1 = selectedObj.selObjID[i-1];
                int nd2 = selectedObj.selObjID[i];

                int nd1Idx = road->indexOfNode( nd1 );
                int nd2Idx = road->indexOfNode( nd2 );

                if( nd1Idx >= 0 && nd2Idx >= 0 ){

                    program->setUniformValue( u_isText, 0 );
                    program->setUniformValue( u_useTex, 2 );
                    glLineWidth( 6 );

                    // check connection
                    bool isValidConnect = false;
                    for(int j=0;j<road->nodes[nd1Idx]->legInfo.size();++j){
                        if( road->nodes[nd1Idx]->legInfo[j]->connectingNode == nd2 ){
                            isValidConnect = true;
                            break;
                        }
                    }
                    if( isValidConnect == true ){
                        isValidConnect = false;
                        for(int j=0;j<road->nodes[nd2Idx]->legInfo.size();++j){
                            if( road->nodes[nd2Idx]->legInfo[j]->connectedNode == nd1 ){
                                isValidConnect = true;
                                break;
                            }
                        }
                    }
                    if( isValidConnect == false ){
                        program->setUniformValue( u_vColor, QVector4D( 1.0, 0.2, 0.0, 1.0 ) );
                    }
                    else{
                        program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 0.2, 1.0 ) );
                    }

                    float x1 = road->nodes[nd1Idx]->pos.x();
                    float y1 = road->nodes[nd1Idx]->pos.y();

                    float x2 = road->nodes[nd2Idx]->pos.x();
                    float y2 = road->nodes[nd2Idx]->pos.y();

                    float dx = x2 - x1;
                    float dy = y2 - y1;
                    float L = sqrt(dx * dx + dy * dy);
                    float angle = atan2( dy, dx );

                    model2World.setTranslation( QVector3D( x1, y1, 0.75 ) );
                    model2World.setScale( QVector3D( L, 1.0, 1.0 ) );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );
                }

                linePoly.array.release();
            }
        }
    }

    model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

    //
    // Axis
    if( axisPoly.isValid == true ){

        axisPoly.array.bind();

        model2World.setTranslation(QVector3D( 0.0, 0.0, 0.0  ));
        model2World.setRotation( QQuaternion( 1.0, 0.0, 0.0, 0.0 ) );

        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

        program->setUniformValue( u_useTex, 0 );
        program->setUniformValue( u_isText, 0 );
        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 0.0, 0.0 ) );

        glLineWidth(3.0);
        glDrawArrays(GL_LINES, 0, 6 );
        glLineWidth(1.0);

        axisPoly.array.release();
    }

    if( textPoly.isTextValid == true ){

        textPoly.textArray.bind();
        textPoly.textBuffer->bind();

        program->setUniformValue( u_useTex, 100 );
        program->setUniformValue( u_isText, 100 );
        program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

        for(int i=0;i<3;++i){

            char str[25];
            if( i == 0 ){
                model2World.setTranslation(QVector3D( 1.0, 0.0, 0.0 ));
                sprintf(str,"x");
            }
            else if( i == 1 ){
                model2World.setTranslation(QVector3D( 0.0, 1.0, 0.0 ));
                sprintf(str,"y");
            }
            else if( i == 2 ){
                model2World.setTranslation(QVector3D( 0.0, 0.0, 1.0 ));
                sprintf(str,"z");
            }

            QQuaternion letterQuat = cameraQuat.conjugated();
            model2World.setRotation( letterQuat );

            program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

            glActiveTexture( GL_TEXTURE0 );

            float x = 0.0;
            float y = 0.0;
            float scale = FONT_SCALE;

            for(unsigned int c=0;c<strlen(str);++c ){

                Character* ch = Characters[ str[c] ];

                GLfloat xpos = x + ch->Bearing.width() * scale;
                GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                float w = ch->Size.width() * scale;
                float h = ch->Size.height() * scale;

                QVector<GLfloat> fontPoly;
                fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                x += ( ch->Advance >> 6 ) * scale;
            }
        }

        textPoly.textBuffer->release();
        textPoly.textArray.release();
    }


    //
    // PedestLane
    //
    if( linePoly.isValid == true ){

        if( pedestPathPointPickFlag == true ){

            model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

            linePoly.array.bind();

            glLineWidth(4.0);

            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_isText, 0 );
            program->setUniformValue( u_vColor, QVector4D( 0.0, 0.5, 1.0, 1.0  ) );

            for(int i=0;i<pedestLanePoints.size()-1;++i){

                model2World.setTranslation( QVector3D( pedestLanePoints[i]->x(),
                                                       pedestLanePoints[i]->y(),
                                                       0.5) );

                float dx = pedestLanePoints[i+1]->x() - pedestLanePoints[i]->x();
                float dy = pedestLanePoints[i+1]->y() - pedestLanePoints[i]->y();

                float len = sqrt( dx * dx + dy * dy );
                model2World.setScale( QVector3D(len,1.0,1.0) );

                float angle = atan2( dy, dx );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                glDrawArrays(GL_LINES, 0, 2 );
            }

            linePoly.array.release();

            if( circlePoly.isValid == true && pedestLanePoints.size() > 0 ){

                circlePoly.array.bind();

                model2World.setTranslation( QVector3D( pedestLanePoints[0]->x(),
                                                       pedestLanePoints[0]->y(),
                                                       0.5) );


                model2World.setRotation( QQuaternion( 1.0, 0.0 , 0.0 , 0.0 )  );

                float w = 1.0;
                model2World.setScale( QVector3D(w,w,1.0) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                program->setUniformValue( u_useTex, 2 );
                program->setUniformValue( u_isText, 0 );
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

                glLineWidth(1.0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                circlePoly.array.release();
            }
        }
        else{

            if( showPedestLaneFlag == true ){

                model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

                for(int n=0;n<road->pedestLanes.size();++n){

                    bool isSelected = false;
                    QList<int> isPointSelected;
                    for(int i=0;i<selectedObj.selObjKind.size();++i){
                        if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE ){
                            if( selectedObj.selObjID[i] == road->pedestLanes[n]->id ){
                                isSelected = true;
                                break;
                            }
                        }
                        else if( selectedObj.selObjKind[i] == _SEL_OBJ::SEL_PEDEST_LANE_POINT ){
                            int modID = selectedObj.selObjID[i];
                            int pedestLaneID = modID / 100;
                            int pedestLanePointIndex = modID - pedestLaneID * 100;

//                            qDebug() << "pedestLaneID = " << pedestLaneID << " pedestLanePointIndex = " << pedestLanePointIndex;

                            if( pedestLaneID == road->pedestLanes[n]->id ){
                                isSelected = true;
                                isPointSelected.append( pedestLanePointIndex );
                            }
                        }
                    }

                    for(int i=0;i<road->pedestLanes[n]->shape.size()-1;++i){

                        linePoly.array.bind();

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );

                        if( isPointSelected.indexOf(i) >= 0 ){
                            glLineWidth(3.0);
                            program->setUniformValue( u_vColor, QVector4D( 0.6, 0.2, 1.0, 1.0  ) );
                        }
                        else{
                            if( isSelected == false ){
                                if( road->pedestLanes[n]->shape[i]->isCrossWalk == true ){
                                    glLineWidth(6.0);
                                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.8, 0.5, 1.0  ) );
                                }
                                else if( road->pedestLanes[n]->shape[i]->canWaitTaxi == true ){
                                    glLineWidth(6.0);
                                    program->setUniformValue( u_vColor, QVector4D( 0.4, 1.0, 0.5, 1.0  ) );
                                }
                                else{
                                    glLineWidth(3.0);
                                    program->setUniformValue( u_vColor, QVector4D( 0.0, 0.5, 1.0, 1.0  ) );
                                }
                            }
                            else{
                                glLineWidth(3.0);
                                program->setUniformValue( u_vColor, QVector4D( 0.3, 0.8, 1.0, 1.0  ) );
                            }
                        }

                        model2World.setTranslation( road->pedestLanes[n]->shape[i]->pos );

                        float len = road->pedestLanes[n]->shape[i]->distanceToNextPos;
                        model2World.setScale( QVector3D(len,1.0,1.0) );

                        float angle = road->pedestLanes[n]->shape[i]->angleToNextPos;
                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                        linePoly.array.release();
                    }

                    for(int i=0;i<road->pedestLanes[n]->shape.size()-1;++i){

                        linePoly.array.bind();

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );

                        glLineWidth(2.0);
                        program->setUniformValue( u_vColor, QVector4D( 0.3, 0.8, 1.0, 1.0  ) );

                        float angle = road->pedestLanes[n]->shape[i]->angleToNextPos;

                        float ct = cos(angle);
                        float st = sin(angle);

                        // Left side
                        QVector3D offset;
                        offset.setX(  -st * road->pedestLanes[n]->shape[i]->width * 0.5 );
                        offset.setY( ct * road->pedestLanes[n]->shape[i]->width * 0.5 );
                        offset.setZ( 0.0 ) ;

                        model2World.setTranslation( road->pedestLanes[n]->shape[i]->pos + offset );

                        float len = road->pedestLanes[n]->shape[i]->distanceToNextPos;
                        model2World.setScale( QVector3D(len,1.0,1.0) );

                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                        // Right side
                        offset.setX( st * road->pedestLanes[n]->shape[i]->width * 0.5 );
                        offset.setY( -ct * road->pedestLanes[n]->shape[i]->width * 0.5 );
                        offset.setZ( 0.0);

                        model2World.setTranslation( road->pedestLanes[n]->shape[i]->pos + offset );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                        linePoly.array.release();
                    }

                    for(int i=0;i<road->pedestLanes[n]->shape.size()-1;++i){

                        if( road->pedestLanes[n]->shape[i]->runOutProb < 0.001 ||
                                road->pedestLanes[n]->shape[i]->marginToRoadForRunOut < 0.001 ){
                            continue;
                        }

                        linePoly.array.bind();

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );

                        glLineWidth(3.0);
                        program->setUniformValue( u_vColor, QVector4D( 0.94, 0.80, 0.1, 1.0  ) );

                        float angle = road->pedestLanes[n]->shape[i]->angleToNextPos;

                        float ct = cos(angle);
                        float st = sin(angle);

                        QVector3D offset;
                        float s = road->pedestLanes[n]->shape[i]->width * 0.5 + road->pedestLanes[n]->shape[i]->marginToRoadForRunOut;
                        if( road->pedestLanes[n]->shape[i]->runOutDirect == 1 ){
                            // Left side
                            offset.setX(  -st * s );
                            offset.setY( ct * s );
                            offset.setZ( 0.0 ) ;
                        }
                        else{
                            // Right side
                            offset.setX( st * s );
                            offset.setY( -ct * s );
                            offset.setZ( 0.0);
                        }

                        model2World.setTranslation( road->pedestLanes[n]->shape[i]->pos + offset );

                        float len = road->pedestLanes[n]->shape[i]->distanceToNextPos;
                        model2World.setScale( QVector3D(len,1.0,1.0) );

                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                        linePoly.array.release();
                    }

                    if( circlePoly.isValid == true ){

                        circlePoly.array.bind();

                        model2World.setTranslation( road->pedestLanes[n]->shape[0]->pos );


                        model2World.setRotation( QQuaternion( 1.0, 0.0 , 0.0 , 0.0 )  );

                        float w = road->pedestLanes[n]->shape[0]->width * 0.5;
                        model2World.setScale( QVector3D(w,w,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );
                        program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );

                        glLineWidth(1.0);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, NODE_CIRCLE_DIV );

                        circlePoly.array.release();
                    }

                    if( textPoly.isTextValid == true && showPedestLaneLabelsFlag == true && showLabelsFlag == true ){

                        textPoly.textArray.bind();
                        textPoly.textBuffer->bind();

                        program->setUniformValue( u_useTex, 100 );
                        program->setUniformValue( u_isText, 100 );
                        program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 1.0, 1.0 ) );

                        char str[25];
                        sprintf(str,"PL%d",road->pedestLanes[n]->id);

                        model2World.setTranslation( QVector3D( road->pedestLanes[n]->shape[0]->pos.x(),
                                                               road->pedestLanes[n]->shape[0]->pos.y(),
                                                               1.0));


                        QQuaternion letterQuat = cameraQuat.conjugated();
                        model2World.setRotation( letterQuat );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glActiveTexture( GL_TEXTURE0 );

                        float x = 0.0;
                        float y = 0.0;
                        float scale = FONT_SCALE;

                        for(unsigned int c=0;c<strlen(str);++c ){

                            Character* ch = Characters[ str[c] ];

                            GLfloat xpos = x + ch->Bearing.width() * scale;
                            GLfloat ypos = y - (ch->Size.height() - ch->Bearing.height()) * scale;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            float w = ch->Size.width() * scale;
                            float h = ch->Size.height() * scale;

                            QVector<GLfloat> fontPoly;
                            fontPoly << 0.0 <<  h   << 2.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
                            fontPoly << 0.0 << 0.0  << 2.0 << 0.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  << 0.0  << 2.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0;
                            fontPoly <<  w  <<  h   << 2.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

                            textPoly.textBuffer->write( 0, fontPoly.constData(), fontPoly.size() * sizeof(GLfloat) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }

                        textPoly.textBuffer->release();
                        textPoly.textArray.release();
                    }
                }
            }
        }


        //
        //  Line Objects
        //
        model2World.setScale( QVector3D(1.0, 1.0, 1.0) );

        for(int i=0;i<lineObj.size();++i){

            for(int j=0;j<lineObj[i]->coord.size();++j){
                lineObj[i]->p[j].setX( (lineObj[i]->coord[j].x() - lineObjCoordInfo.center.x()) * lineObjCoordInfo.scale_x );
                lineObj[i]->p[j].setY( (lineObj[i]->coord[j].y() - lineObjCoordInfo.center.y()) * lineObjCoordInfo.scale_y );
            }

            linePoly.array.bind();

            glLineWidth(8.0);

            program->setUniformValue( u_useTex, 2 );
            program->setUniformValue( u_isText, 0 );

            if( lineObj[i]->color == 0 ){
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 1 ){
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.5, 1.0 ) );
            }
            else if( lineObj[i]->color == 2 ){
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 1.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 3 ){
                program->setUniformValue( u_vColor, QVector4D( 0.5, 0.0, 1.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 4 ){
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.0, 1.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 5 ){
                program->setUniformValue( u_vColor, QVector4D( 0.0, 0.5, 1.0, 1.0 ) );
//                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.0, 0.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 6 ){
                program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 1.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 7 ){
                program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 0.5, 1.0 ) );
            }
            else if( lineObj[i]->color == 8 ){
                program->setUniformValue( u_vColor, QVector4D( 0.0, 1.0, 0.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 9 ){
                program->setUniformValue( u_vColor, QVector4D( 0.5, 1.0, 0.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 10 ){
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.0, 1.0 ) );
            }
            else if( lineObj[i]->color == 11 ){
                program->setUniformValue( u_vColor, QVector4D( 1.0, 1.0, 0.5, 1.0 ) );
            }

            int color = lineObj[i]->color;

            if( color == 7 | color == 4  || color == 11 || color == 5 || color == 1 || color == 2 || color == 3 ){

                if( lineObj[i]->p.size() > 2 ){
                    model2World.setTranslation( QVector3D( lineObj[i]->p[0].x(),
                                                           lineObj[i]->p[0].y(),
                                                           0.5) );

                    QPointF diff = lineObj[i]->p[2] - lineObj[i]->p[0];
                    float len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                    model2World.setScale( QVector3D(len,1.0,1.0) );

                    float angle = atan2( diff.y(), diff.x() );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );


                    for(int j=2;j<lineObj[i]->p.size()-1;++j){

                        model2World.setTranslation( QVector3D( lineObj[i]->p[j].x(),
                                                               lineObj[i]->p[j].y(),
                                                               0.5) );

                        QPointF diff = lineObj[i]->p[j+1] - lineObj[i]->p[j];
                        float len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                        model2World.setScale( QVector3D(len,1.0,1.0) );

                        float angle = atan2( diff.y(), diff.x() );
                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        glDrawArrays(GL_LINES, 0, 2 );

                    }

                    int lidx = lineObj[i]->p.size() - 1;
                    model2World.setTranslation( QVector3D( lineObj[i]->p[lidx].x(),
                                                           lineObj[i]->p[lidx].y(),
                                                           0.5) );

                    diff = lineObj[i]->p[1] - lineObj[i]->p[lidx];
                    len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                    model2World.setScale( QVector3D(len,1.0,1.0) );

                    angle = atan2( diff.y(), diff.x() );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );

                }
                else{

                    model2World.setTranslation( QVector3D( lineObj[i]->p[0].x(),
                                                           lineObj[i]->p[0].y(),
                                                           0.5) );

                    QPointF diff = lineObj[i]->p[1] - lineObj[i]->p[0];
                    float len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                    model2World.setScale( QVector3D(len,1.0,1.0) );

                    float angle = atan2( diff.y(), diff.x() );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );

                }

            }
            else if( color == 10 ){
                for(int j=0;j<lineObj[i]->p.size()-1;++j){

                    model2World.setTranslation( QVector3D( lineObj[i]->p[j].x(),
                                                           lineObj[i]->p[j].y(),
                                                           0.5) );

                    QPointF diff = lineObj[i]->p[j+1] - lineObj[i]->p[j];
                    float len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                    model2World.setScale( QVector3D(len,1.0,1.0) );

                    float angle = atan2( diff.y(), diff.x() );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );

                }
            }
            else if( color == 6 ||color == 8 || color == 9  ){
                for(int j=0;j<lineObj[i]->p.size();j+=2){

                    model2World.setTranslation( QVector3D( lineObj[i]->p[j].x(),
                                                           lineObj[i]->p[j].y(),
                                                           0.5) );

                    QPointF diff = lineObj[i]->p[j+1] - lineObj[i]->p[j];
                    float len = sqrt( diff.x() * diff.x() + diff.y() * diff.y() );

                    model2World.setScale( QVector3D(len,1.0,1.0) );

                    float angle = atan2( diff.y(), diff.x() );
                    model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );
                    program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                    glDrawArrays(GL_LINES, 0, 2 );

                }
            }


            glLineWidth(1.0);

            linePoly.array.release();
        }
    }
}


void GraphicCanvas::resizeGL(int w, int h)
{
    currentWidth  = w;
    currentHeight = h;
    aspectRatio = float(w) / float(h);

    projection.setToIdentity();
    if( isOrthogonal == true ){
        float nearDist = 0.1;
        float farDist = 10000.0;
        float t = fabs(Z_eye) * tan(45.0f / 2.0f * 0.017452f);
        float r = t * aspectRatio;
        projection.ortho( -r, r, -t, t, nearDist, farDist );
    }
    else{
        projection.perspective( fieldOfView, aspectRatio, 0.1, 100000.0 );
    }

    sx = 1.0 / float(w);
    sy = 1.0 / float(h);
}


void GraphicCanvas::SetNumberKeyPressed(int key)
{
    if( selectedObj.selObjKind.size() == 0 ){
        numberKeyPressed = -1;
        return;
    }

    if( selectedObj.selObjKind.size() > 0 &&
            selectedObj.selObjKind[0] == _SEL_OBJ::SEL_NODE ){
        int idx = road->indexOfNode( selectedObj.selObjID[0] );
        if( idx >= 0 ){
            if( key > 0 && key <= road->nodes[idx]->legInfo.size()){
                numberKeyPressed = key;
                update();
            }
        }
    }
}


void GraphicCanvas::SetNodePickMode(int oNode,int dNode)
{
    pedestPathPointPickFlag = false;
    roadBoundaryPointPickFlag = false;
    nodePickModeFlag = true;

    selectedObj.selObjKind.clear();
    selectedObj.selObjID.clear();
    numberKeyPressed = -1;

    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE_ROUTE_PICK );
    selectedObj.selObjID.append( oNode );

    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE_ROUTE_PICK );
    selectedObj.selObjID.append( dNode );

    update();
}


void GraphicCanvas::ResetNodePickMode()
{
    nodePickModeFlag = false;
    update();
}


void GraphicCanvas::SetPedestLanePointPickMode()
{
    nodePickModeFlag = false;
    roadBoundaryPointPickFlag = false;
    pedestPathPointPickFlag = true;

    for(int i=0;i<pedestLanePoints.size();++i){
        delete pedestLanePoints[i];
    }
    pedestLanePoints.clear();

    update();
}


void GraphicCanvas::ResetPedestLanePointPickMode()
{
    if( pedestPathPointPickFlag == true && pedestLanePoints.size() > 1 ){
        emit PedestLanePointPicked();
    }
    pedestPathPointPickFlag = false;
    update();
}


void GraphicCanvas::RemovePickedPedestLanePoint()
{
    if( pedestPathPointPickFlag == true ){
        if( pedestLanePoints.size() > 1 ){
            pedestLanePoints.removeLast();
            update();
        }
    }
}


void GraphicCanvas::SetRoadBoundaryPointPickMode()
{
    roadBoundaryPointPickFlag = true;
    nodePickModeFlag = false;
    pedestPathPointPickFlag = false;

    for(int i=0;i<roadBoundaryPoints.size();++i){
        delete roadBoundaryPoints[i];
    }
    roadBoundaryPoints.clear();

    update();
}

void GraphicCanvas::ResetRoadBoundaryPointPickMode()
{
    if( roadBoundaryPointPickFlag == true && roadBoundaryPoints.size() > 1 ){
        emit RoadBoundaryPointPicked();
    }
    roadBoundaryPointPickFlag = false;
    update();
}


void GraphicCanvas::RemovePickedRoadBoundaryPoint()
{
    if( roadBoundaryPointPickFlag == true ){
        if( roadBoundaryPoints.size() > 1 ){
            roadBoundaryPoints.removeLast();
            update();
        }
    }
}


void GraphicCanvas::LoadMapImage(struct baseMapImage* bmi)
{
    // Load Image
    QImage map;

    bmi->isValid = false;

    QString PathToFile = bmi->path;
    if( PathToFile.endsWith("/") == false ){
        PathToFile += QString("/");
    }

    QString testname = PathToFile + bmi->filename;

//    qDebug() << "Loading " << testname;

    if( map.load(testname) == false ){

        qDebug() << "Loading " << testname;
        qDebug() << "Failed.";
        bool found = false;

        QStringList divTestName = testname.replace("\\","/").split("/");
        QString imageFileName = QString( divTestName.last() );

        for(int i=0;i<imageFilePathFolders.size();++i){

            QString PathToFile = QString(imageFilePathFolders[i]);
            if( PathToFile.endsWith("/") == false ){
                PathToFile += QString("/");
            }

            QString tmpImageFilePath = PathToFile + imageFileName;

//            qDebug() << "Loading " << tmpImageFilePath;
            if( map.load(tmpImageFilePath) == false ){
                continue;
            }
            else{
                qDebug() << "alt. = " << tmpImageFilePath;
                bmi->path = PathToFile;
                found = true;
                break;
            }

        }

        if( found == false ){

            QString newImageFilePath = QFileDialog::getExistingDirectory(this,"SEdit","Select Folder where Image Files stored");
            if( newImageFilePath.isNull() || newImageFilePath.isEmpty() ){
                // Cancel
                return;
            }
            if( newImageFilePath.endsWith("/") == false ){
                newImageFilePath += QString("/");
            }

            QString tmpImageFilePath = newImageFilePath + imageFileName;

//            qDebug() << "Loading " << tmpImageFilePath;
            if( map.load(tmpImageFilePath) == false ){
                qDebug() << "Loading " << tmpImageFilePath;
                qDebug() << "Failed.";
                return;
            }
            else{
                qDebug() << "alt. = " << tmpImageFilePath;
                bmi->path = newImageFilePath;
                imageFilePathFolders.append( newImageFilePath );
            }
        }
    }
    else{
        qDebug() << "Loaded: " << testname;
    }


    glGenTextures(1, &(bmi->textureID));
    glBindTexture(GL_TEXTURE_2D, bmi->textureID);

    int wi = map.width();
    int hi = map.height();

    //qDebug() << "width = " << wi << " height = " << hi;


    bmi->halfWidth = wi * 0.5;
    bmi->halfHeight = hi * 0.5;

    int wi2 = 2;
    while( wi2 * 2 <= wi )
        wi2 *= 2;

    if( wi2 > 2048 )
        wi2 = 2048;

    int hi2 = 2;
    while( hi2 * 2 <= hi )
            hi2 *= 2;

    if( hi2 > 2048 )
        hi2 = 2048;


    // Make resolution squar size
    if( hi2 < wi2 ){
        hi2 = wi2;
    }
    else if( hi2 > wi2 ){
        wi2 = hi2;
    }

    //qDebug() << "mod width = " << wi2 << " mod height = " << hi2;


    GLubyte *bits;
    int tsz = wi2 * hi2 * 4;
    bits = new GLubyte [tsz];

    tsz = 0;
    for(int i=0;i<wi2;++i){
        int x = (int)((i / (float)wi2) * (float)wi);
        for(int j=0;j<hi2;++j){
            int y = (int)((j / (float)hi2) * (float)hi);
            QRgb pix = map.pixel(x,y);
            bits[tsz++] = (GLubyte)qRed(pix);
            bits[tsz++] = (GLubyte)qGreen(pix);
            bits[tsz++] = (GLubyte)qBlue(pix);
            bits[tsz++] = 0xff;
        }
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA , wi2 , hi2,  0 , GL_RGBA , GL_UNSIGNED_BYTE , bits );

    delete [] bits;

    glBindTexture(GL_TEXTURE_2D, 0);

    bmi->isValid = true;

//    qDebug() << "Image loaded successfully.";
}


void GraphicCanvas::DeleteMapImage(struct baseMapImage *bmi )
{
    glDeleteTextures(1, &(bmi->textureID));
    delete bmi;
}


void GraphicCanvas::SetNodeSelected(int node)
{
    qDebug() << "[GraphicCanvas::SetNodeSelected] node = " << node;

    selectedObj.selObjKind.clear();
    selectedObj.selObjID.clear();

    nodePickModeFlag        = false;
    pedestPathPointPickFlag = false;

    selectedObj.selObjKind.append( _SEL_OBJ::SEL_NODE );
    selectedObj.selObjID.append( node );

    update();
}


void GraphicCanvas::SetScenarioPickMode(int m)
{
    qDebug() << "[GraphicCanvas::SetScenarioPickMode] mode = " << m;

    activateWindow();

    scenarioPickMode = m;
}


