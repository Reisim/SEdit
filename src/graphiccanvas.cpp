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

    showNodeLabelsFlag          = true;
    showLaneLabelsFlag          = true;
    showTrafficSignalLabelsFlag = true;
    showStopLineLabelsFlag      = true;
    showPedestLaneLabelsFlag    = true;
    showLabelsFlag              = true;

    selectNodeFlag          = true;
    selectLaneFlag          = true;
    selectTrafficSignalFlag = true;
    selectStopLineFlag      = true;
    selectPedestLaneFlag    = true;

    LaneListFlag           = false;
    laneListIndex          = 0;
    RelatedLanesFlag       = false;
    RouteLaneListFlag      = true;

    colorLaneBySpeedLimitFlag  = false;
    colorLaneByActualSpeedFlag = false;

    nodePickModeFlag = false;
    pedestPathPointPickFlag = false;

    mousePressed = false;


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


    setMouseTracking(true);
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

}



void GraphicCanvas::paintGL()
{
    if( nodePickModeFlag == true || pedestPathPointPickFlag == true ){
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

            linePoly.array.bind();


            program->setUniformValue( u_isText, 0 );


            if( isSelected == true ){
                program->setUniformValue( u_useTex, 2 );
                glLineWidth( laneDrawWidth * 2 );
                program->setUniformValue( u_vColor, QVector4D( 1.0, 0.5, 0.5, 1.0 ) );
            }
            else{

                if( isRelatedLane == true ){
                    program->setUniformValue( u_useTex, 2 );
                    glLineWidth( laneDrawWidth );
                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.92, 0.66, 1.0 ) );
                }
                else{

                    if( colorLaneByActualSpeedFlag == true ){

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

            glLineWidth(2.0);

            if( road->lanes[i]->previousLanes.size() == 0 ){

                float c = road->lanes[i]->shape.derivative.first()->x();
                float s = road->lanes[i]->shape.derivative.first()->y();

                model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.first()->x() - s * 1.25,
                                                       road->lanes[i]->shape.pos.first()->y() + c * 1.25,
                                                       road->lanes[i]->shape.pos.first()->z()) );

                model2World.setScale( QVector3D(2.5,1.0,1.0) );

                float angle = atan2( -c, s );
                model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );
                program->setUniformValue( u_vColor, QVector4D( 0.5, 0.3, 0.3, 1.0 ) );

                glDrawArrays(GL_LINES, 0, 2 );
            }

            {
                float c = road->lanes[i]->shape.derivative.last()->x();
                float s = road->lanes[i]->shape.derivative.last()->y();

                model2World.setTranslation( QVector3D( road->lanes[i]->shape.pos.last()->x() - s * 1.25,
                                                       road->lanes[i]->shape.pos.last()->y() + c * 1.25,
                                                       road->lanes[i]->shape.pos.last()->z()) );

                model2World.setScale( QVector3D(2.5,1.0,1.0) );

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
                            GLfloat ypos = y;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }
                    }
                }

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
                else{
                    glLineWidth(3.0);
                }
                glDrawArrays(GL_LINE_LOOP, 0, NODE_CIRCLE_DIV );
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

                    program->setUniformValue( u_useTex, 0 );
                    program->setUniformValue( u_isText, 0 );

                    glLineWidth(1.0);
                    if( isSelected == true && numberKeyPressed > 0 && numberKeyPressed == j + 1 ){
                        glLineWidth(4.0);
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
                                                               0.0) );

                        float dx = road->nodes[i]->stopLines[j]->rightEdge.x() - road->nodes[i]->stopLines[j]->leftEdge.x();
                        float dy = road->nodes[i]->stopLines[j]->rightEdge.y() - road->nodes[i]->stopLines[j]->leftEdge.y();
                        float L = sqrt( dx * dx + dy * dy );
                        float angle = atan2( dy, dx );

                        model2World.setRotation( QQuaternion( cos(angle*0.5), 0.0 , 0.0 , sin(angle*0.5) ) );

                        model2World.setScale( QVector3D(L,1.0,1.0) );

                        program->setUniformValue( u_modelToCamera,  world2camera * model2World.getWorldMatrix() );

                        program->setUniformValue( u_useTex, 0 );
                        program->setUniformValue( u_isText, 0 );

                        glLineWidth(4.0);
                        if( isSLSelected == true ){
                            glLineWidth(8.0);
                        }
                        glDrawArrays(GL_LINES, 0, 2 );
                        glLineWidth(1.0);

                        linePoly.array.release();
                    }
                }
            }


            if( textPoly.isTextValid == true ){

                textPoly.textArray.bind();

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
                            GLfloat ypos = y;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

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
                                GLfloat ypos = y;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

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
                                GLfloat ypos = y;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

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
                                GLfloat ypos = y;
                                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                                x += ( ch->Advance >> 6 ) * scale;
                            }
                        }
                    }
                }

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
                    glLineWidth( 3 );

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

                    model2World.setTranslation( QVector3D( x1, y1, 0.25 ) );
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
                GLfloat ypos = y;
                program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                x += ( ch->Advance >> 6 ) * scale;
            }
        }

        textPoly.textArray.release();
    }


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
                    int isPointSelected = -1;
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
                                isPointSelected = pedestLanePointIndex;
                                break;
                            }
                        }
                    }

                    for(int i=0;i<road->pedestLanes[n]->shape.size()-1;++i){

                        linePoly.array.bind();

                        program->setUniformValue( u_useTex, 2 );
                        program->setUniformValue( u_isText, 0 );

                        if( i == isPointSelected ){
                            glLineWidth(3.0);
                            program->setUniformValue( u_vColor, QVector4D( 0.6, 0.2, 1.0, 1.0  ) );
                        }
                        else{
                            if( isSelected == false ){
                                if( road->pedestLanes[n]->shape[i]->isCrossWalk == true ){
                                    glLineWidth(6.0);
                                    program->setUniformValue( u_vColor, QVector4D( 1.0, 0.8, 0.5, 1.0  ) );
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
                            GLfloat ypos = y;
                            program->setUniformValue( u_letterPos, QVector3D(xpos, ypos, 0.0) );

                            glBindTexture( GL_TEXTURE_2D, ch->TextureID );

                            glDrawArrays(GL_QUADS, 0, 4 * sizeof(GLfloat) );

                            x += ( ch->Advance >> 6 ) * scale;
                        }

                        textPoly.textArray.release();
                    }
                }
            }
        }


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

        QStringList divTestName = testname.split("/");
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
                bmi->path = newImageFilePath;
                imageFilePathFolders.append( newImageFilePath );
            }
        }
    }


    glGenTextures(1, &(bmi->textureID));
    glBindTexture(GL_TEXTURE_2D, bmi->textureID);

    int wi = map.width();
    int hi = map.height();

    bmi->halfWidth = wi * 0.5;
    bmi->halfHeight = hi * 0.5;

    int wi2 = 2;
    while( wi2 <= wi )
        wi2 *= 2;

    if( wi2 > 2048 )
        wi2 = 2048;

    int hi2 = 2;
    while( hi2 <= hi )
            hi2 *= 2;

    if( hi2 > 2048 )
        hi2 = 2048;

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


