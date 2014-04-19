#include <QtGui>
#include <QtXml>
#include <iostream>

#include "robotmodelcfg.h"
#include "debug.h"

RobotModelCfg::RobotModelCfg()
{
    m_robotname = (char *)malloc(MAX_FILENAME_LEN * sizeof(char *));

    for (unsigned char i = 0; i < MAX_LEN; i++)
    {
        m_modelName[i] = (char *)malloc(MAX_FILENAME_LEN * sizeof(char *));
        m_filename[i] = (char *)malloc(MAX_FILENAME_LEN * sizeof(char *)) ;
        nodeAxisName[i] = (char *)malloc(MAX_FILENAME_LEN * sizeof(char *));
    }

    InitCfg();

}

RobotModelCfg::~RobotModelCfg()
{
    for (unsigned char i = 0; i < sizeof(m_filename)/sizeof(const char *); i++)
    {
        free(m_filename[i]);
        free(m_modelName[i]);
        free(nodeAxisName[i]);
    }
}
void RobotModelCfg::debugInfo()
{
    for (unsigned char i = 0; i < AxisNum; i++)
    {
        qDebug("[%d]m_filename:%s, m_modelName:%s, nodeAxisName:%s",
               i, m_filename[i], m_modelName[i], nodeAxisName[i]);
        qDebug("[%d]rx:%12f,ry:%12f,rz:%12f", i, axisRotation[i][0], axisRotation[i][1], axisRotation[i][2] );
        qDebug("[%d] x:%12f, y:%12f, z:%12f", i, axisPosition[i][0], axisPosition[i][1], axisPosition[i][2] );
    }
    qDebug("m_scale = %f ", m_scale);
}
const char * RobotModelCfg::get_filename(unsigned int index)
{
    return m_filename[index];
}

void RobotModelCfg::set_filename(const char *inFilename, unsigned int index)
{
    char * pchar = m_filename[index];
    snprintf(pchar, MAX_FILENAME_LEN, inFilename);
}

void RobotModelCfg::set_modelName(const char *name, unsigned int index)
{
    char * pchar = m_modelName[index];
    snprintf(pchar, MAX_FILENAME_LEN, name);
}

void RobotModelCfg::set_nodeAxisName(const char *name, unsigned int index)
{
    char * pchar = nodeAxisName[index];
    snprintf(pchar, MAX_FILENAME_LEN, name);
}

void RobotModelCfg::set_axisRotation(float rx, float ry, float rz, unsigned int index)
{
    axisRotation[index][0] = rx;
    axisRotation[index][1] = ry;
    axisRotation[index][2] = rz;
}

void RobotModelCfg::set_axisPosition(float x, float y, float z, unsigned int index)
{
    axisPosition[index][0] = x;
    axisPosition[index][1] = y;
    axisPosition[index][2] = z;
}

void RobotModelCfg::set_axisRotAttr(float min, float max, unsigned int index)
{
    m_axisRotAttr[index][0] = min;
    m_axisRotAttr[index][1] = max;
}

bool RobotModelCfg::updateCfgFromXml(const char *xmlFilename)
{
    Q_ASSERT(NULL != xmlFilename);

    if (readfile(xmlFilename))
    {
        emit sigCfgChanged(this);
        return true;
    }

    return false;
}

bool RobotModelCfg::saveCfgtoXml(const char *xmlFilename)
{
    Q_ASSERT(NULL != xmlFilename);

    //打开文件
    QFile file(xmlFilename);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);//格式输出，也就是会有标签的缩进
    xmlWriter.writeStartDocument();//开始进行 XML 文档的输出,这个函数会写下 <?xml version="1.0" encoding="UTF-8"?>
    xmlWriter.writeStartElement("robot_definition"); //根节点
        xmlWriter.writeStartElement("robot_dk"); //写下一个 entry 的开始标签
        xmlWriter.writeAttribute("name", "FANUC_M710iC_50"); //然后给这个标签一个属性 term，属性值是 of vectors。
            int axisNum = getAxisNum();
            for(int i = 0; i < axisNum; i++)
            {
                xmlWriter.writeStartElement("axis");
                xmlWriter.writeAttribute("id", get_nodeAxisName(i));
                xmlWriter.writeAttribute("x", QString::number(get_axisPosition(i)[0]));
                xmlWriter.writeAttribute("y", QString::number(get_axisPosition(i)[1]));
                xmlWriter.writeAttribute("z", QString::number(get_axisPosition(i)[2]));
                xmlWriter.writeAttribute("rx", QString::number(get_axisRotation(i)[0]));
                xmlWriter.writeAttribute("ry", QString::number(get_axisRotation(i)[1]));
                xmlWriter.writeAttribute("rz", QString::number(get_axisRotation(i)[2]));
                xmlWriter.writeAttribute("limsup", "a");
                xmlWriter.writeAttribute("liminf", "a");
                xmlWriter.writeAttribute("step", "a");
                xmlWriter.writeEndElement();
            }
        xmlWriter.writeEndElement(); //关闭标签

        xmlWriter.writeStartElement("robot_geometry"); //写下一个 entry 的开始标签
            for(int i = 0; i < axisNum; i++)
            {
                QFileInfo xmlfile(get_filename(i));
                xmlWriter.writeStartElement("geometry");
                xmlWriter.writeAttribute("geo", xmlfile.fileName());
                xmlWriter.writeAttribute("name", get_rootName(i));
                xmlWriter.writeEndElement();
            }
        xmlWriter.writeEndElement(); //关闭标签

        xmlWriter.writeStartElement("CAD_scale"); //写下一个 entry 的开始标签
        xmlWriter.writeAttribute("FACTOR", "1"); //然后给这个标签一个属性 term，属性值是 of vectors。
        xmlWriter.writeEndElement(); //关闭标签
    xmlWriter.writeEndElement(); //关闭标签
    xmlWriter.writeEndDocument(); //这个 XML 文档已经写完。

    //事实上，file离开作用域之后，便会自动关闭。
    file.close();

    return true;
}

void RobotModelCfg::InitCfg()
{
    m_scale = 1.0;
    AxisNum = 0;
    memset(m_robotname, 0, MAX_FILENAME_LEN * sizeof(char *));

    for (unsigned char i = 0; i < MAX_LEN; i++)
    {
        axisRotation[i][0] = 0.0;
        axisRotation[i][1] = 0.0;
        axisRotation[i][2] = 0.0;

        axisPosition[i][0] = 0.0;
        axisPosition[i][1] = 0.0;
        axisPosition[i][2] = 0.0;

        m_axisRotAttr[i][0] = 0.0;
        m_axisRotAttr[i][1] = 0.0;

        memset(m_modelName[i], 0, MAX_FILENAME_LEN * sizeof(char *));
        memset(m_filename[i], 0, MAX_FILENAME_LEN * sizeof(char *));
        memset(nodeAxisName[i], 0, MAX_FILENAME_LEN * sizeof(char *));
    }

    return;
}


bool RobotModelCfg::readfile(const char *filename)
{
    //打开文件
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Error: Cannot read file %1: %2")
                             .arg(qPrintable(filename))
                             .arg(qPrintable(file.errorString())));
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;

    //QDom解析
    QDomDocument doc;
    if (!doc.setContent(&file, false, &errorStr, &errorLine,
                        &errorColumn))
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Error: Parse error at line %1, column %2:%3")
                             .arg(errorLine).arg(errorColumn).arg(qPrintable(errorStr)));
        return false;
    }

    //获取并验证doc根元素名称
    QDomElement root = doc.documentElement();
    if (root.tagName() != "robot_definition")
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Error: Not a robot_definition file"));
        return false;
    }

    //解析并验证根元素,错误提示在被调用函数弹出。
    if (!parseRootElement(root, filename))
    {
        return false;
    }

    return true;
}

//此函数可以用面向对象的模式重构。
bool RobotModelCfg::parseRootElement(const QDomElement &element, const QString &filename)
{
    QDomElement elementTmp;
    QDomNode child = element.firstChild();
    bool ret = true;

    while (!child.isNull())
    {
        elementTmp = child.toElement();
        if (elementTmp.tagName() == "robot_dk")
        {
            if (!parseRobotDKElement(elementTmp))
            {
                ret = false;
                break;
            }
        }
        else if (elementTmp.tagName() == "robot_geometry")
        {
            QStringList filenameList;

            //获取 robot_geometry 结点数据
            parseGeometryElement(filenameList, elementTmp, "geometry", "geo");

            //把文件名list，转为文件绝对路径List
            QFileInfo xml(filename);
            QString filepath = xml.absolutePath() + QDir::separator();
            QStringList fullFilenameList;
            foreach(QString filename, filenameList)
            {
                fullFilenameList.append(filepath + filename);
            }

            //校验 robot_geometry结点信息，并保存
            if (!processGeometryData(fullFilenameList))
            {
                ret = false;
                break;
            }

            //解析name字段
            QStringList modelNameList;
            parseGeometryElement(modelNameList, elementTmp, "geometry", "name");

            //并保存模型名称
            unsigned int index = 0;
            foreach(QString modelName, modelNameList)
            {
                set_modelName(qPrintable(modelName),index++);
            }

        }
        else if (elementTmp.tagName() == "CAD_base")
        {
        }
        else if (elementTmp.tagName() == "CAD_scale")
        {
            if (!parseScaleElement(elementTmp))
            {
                ret =false;
                break;
            }
        }
        else
        {
        }

        child = child.nextSibling();
    }

    return ret;
}

bool RobotModelCfg::processGeometryData(const QStringList &fullFilenameList)
{
    if (fullFilenameList.length() != this->AxisNum)
    {
        QMessageBox::warning(0,QObject::tr("DOM Parser"),QObject::tr("geometry(%1) is not eaql to axis(%2)")
                             .arg(fullFilenameList.length()).arg(this->AxisNum));
        return false;
    }

    //判断结点文件是否存在,以及文件大小
    foreach(QString fullFilename, fullFilenameList)
    {
        if (!QFile::exists(fullFilename))
        {
            QMessageBox::warning(0, QObject::tr("DOM Parser"),
                                 QObject::tr("Error: %1 file not exits!")
                                 .arg(fullFilename));
            return false;
        }

        if (MAX_FILENAME_LEN < fullFilename.length())
        {
            QMessageBox::warning(0, QObject::tr("DOM Parser"),
                                 QObject::tr("Error: %1 fullFilename length is longer than %2!")
                                 .arg(fullFilename).arg(MAX_FILENAME_LEN));
            return false;
        }

    }

    //保存数据
    quint8 index =0;
    foreach(QString fullFilename, fullFilenameList)
    {
        set_filename(qPrintable(fullFilename), index);
        index++;
    }

    return true;
}

/*************************************************
Function: // 函数名称
Description: // 函数功能、性能等的描述

Input: // 输入参数说明，包括每个参数的作
// 用、取值说明及参数间关系。
Output: // 对输出参数的说明。
Return: // 函数返回值的说明
Author: zhangjiankun
Others: // 其它说明
*************************************************/
void RobotModelCfg::parseGeometryElement(QStringList &filenameList, const QDomElement &element, const QString &tagName, const QString &attriName)
{
    QDomNode child = element.firstChild();

    while (!child.isNull())
    {
        QDomElement elementTmp = child.toElement();
        if (elementTmp.tagName() == tagName)
        {
            QString filename = /*filepath +*/ elementTmp.attribute(attriName);
            filenameList.append(filename);
        }
        child = child.nextSibling();
    }

    return ;
}

bool RobotModelCfg::parseRobotDKElement(const QDomElement &element)
{
    QDomNode child = element.firstChild();

    this->AxisNum = 0;
    while (!child.isNull())
    {
        QDomElement elementTmp = child.toElement();
        if (elementTmp.tagName() == "axis")
        {
            float min = elementTmp.attribute("liminf").toFloat();
            float max = elementTmp.attribute("limsup").toFloat();
            set_nodeAxisName(qPrintable(elementTmp.attribute("id")), this->AxisNum);
            set_axisRotation(elementTmp.attribute("rx").toFloat(), elementTmp.attribute("ry").toFloat(),
                             elementTmp.attribute("rz").toFloat(), this->AxisNum);
            set_axisPosition(elementTmp.attribute("x").toFloat(), elementTmp.attribute("y").toFloat(),
                             elementTmp.attribute("z").toFloat(), this->AxisNum);
            if (max > min)
            {
                set_axisRotAttr(min, max, this->AxisNum);
            }

            this->AxisNum++;
            if (MAX_LEN < this->AxisNum)
            {
                QMessageBox::warning(0, QObject::tr("DOM Parser"),
                                     QObject::tr("Error: There are too many axises(%1) in the robot model file!")
                                     .arg(this->AxisNum));

                return false;
            }
        }
        child = child.nextSibling();
    }

    return true;
}

bool RobotModelCfg::parseScaleElement(const QDomElement &element)
{
    if ( 0.01 > element.attribute("FACTOR").toFloat())
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Error: scale FACTOR is less then 0.01!"));
        return false;
    }
    else
    {
        set_scale(element.attribute("FACTOR").toFloat());
        return true;
    }
}

