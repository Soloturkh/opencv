// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2019 Intel Corporation

#include <backends/cpu/serialization.hpp>

#include <opencv2/gapi/gtype_traits.hpp>

#include "compiler/passes/passes.hpp" // dump_dot

#include <ade/util/zip_range.hpp>

namespace cv {
namespace gimpl {
namespace serialization {
//namespace {

// FIXME? make a method of GSerialized?
void putData(GSerialized& s, const GModel::ConstGraph& cg, const ade::NodeHandle nh)
{
    const auto gdata = cg.metadata(nh).get<gimpl::Data>();
    Data d{RcDesc{gdata.shape, gdata.rc}, gdata.meta};

    auto dataInSerialized = ade::util::find(s.m_datas, d);
    // FIXME:
    // put meta check here
    if (s.m_datas.end() == dataInSerialized)
    {
        s.m_datas.push_back(d);
    }
}

void putOp(GSerialized& s, const GModel::ConstGraph& cg, const ade::NodeHandle nh)
{
    const auto& op = cg.metadata(nh).get<gimpl::Op>();

    serialization::Op sop{Kernel{op.k.name, op.k.tag}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
    sop.outs.resize(op.outs.size());

    for(size_t i=0; i < op.args.size(); ++i)
    {
        if(op.args[i].kind == detail::ArgKind::GOBJREF)
        {
            const gimpl::RcDesc &rc = op.args[i].get<gimpl::RcDesc>();
            RcDesc src = {rc.shape, rc.id};
            sop.kind.push_back((int)detail::ArgKind::GOBJREF);
            sop.opaque_kind.push_back((int)detail::OpaqueKind::UNSUPPORTED);
            sop.ins.push_back(src);
        }
        else if(op.args[i].kind == detail::ArgKind::OPAQUE)
        {
            sop.kind.push_back((int)detail::ArgKind::OPAQUE);
            sop.opaque_kind.push_back((int)op.args[i].opaque_kind);
            switch (op.args[i].opaque_kind)
            {
            case detail::OpaqueKind::INT:
                std::cout << "putOp    int " << op.args[i].get<int>() << std::endl;
                sop.opaque_ints.push_back(op.args[i].get<int>());
                break;
            case detail::OpaqueKind::DOUBLE:
                std::cout << "putOp    double " << op.args[i].get<double>() << std::endl;
                sop.opaque_doubles.push_back(op.args[i].get<double>());
                break;
            case detail::OpaqueKind::CV_SIZE:
                std::cout << "putOp    cv::Size " << op.args[i].get<cv::Size>().width << "x"
                          << op.args[i].get<cv::Size>().height << std::endl;
                sop.opaque_cvsizes.push_back(op.args[i].get<cv::Size>());
                break;
            case detail::OpaqueKind::BOOL:
                std::cout << "putOp    bool " << op.args[i].get<bool>() << std::endl;
                sop.opaque_bools.push_back(op.args[i].get<bool>());
                break;
            case detail::OpaqueKind::CV_SCALAR:
                std::cout << "putOp    cv::Scalar " << op.args[i].get<cv::Scalar>()[0] << " "
                          << op.args[i].get<cv::Scalar>()[1] << " "
                          << op.args[i].get<cv::Scalar>()[2] << " "
                          << op.args[i].get<cv::Scalar>()[3] << " "
                          << std::endl;
                sop.opaque_cvscalars.push_back(op.args[i].get<cv::Scalar>());
                break;
            case detail::OpaqueKind::CV_POINT:
                std::cout << "putOp    cv::Point " << op.args[i].get<cv::Point>().x << " "
                          << op.args[i].get<cv::Point>().y << " "
                          << std::endl;
                sop.opaque_cvpoints.push_back(op.args[i].get<cv::Point>());
                break;
            case detail::OpaqueKind::CV_MAT:
                std::cout << "putOp    cv::Mat " << op.args[i].get<cv::Mat>().rows << " "
                          << op.args[i].get<cv::Mat>().cols << " "
                          << op.args[i].get<cv::Mat>().type() << " "
                          << std::endl;
                sop.opaque_cvmats.push_back(op.args[i].get<cv::Mat>());
                break;
            case detail::OpaqueKind::CV_RECT:
                std::cout << "putOp    cv::Rect " << op.args[i].get<cv::Rect>().x << " "
                          << op.args[i].get<cv::Rect>().y << " "
                          << op.args[i].get<cv::Rect>().width << " "
                          << op.args[i].get<cv::Rect>().height << " "
                          << std::endl;
                sop.opaque_cvrects.push_back(op.args[i].get<cv::Rect>());
                break;
            default:
                std::cout << "putOp    OpaqueKind::UNSUPPORTED" << std::endl;
            }
        }
        else
        {
            util::throw_error(std::logic_error("Unexpected ArgKind: expected GOBJREF or OPAQUE"));
        }
    }

    for (size_t i = 0; i < op.outs.size(); i++)
    {
        const auto rc = op.outs[i];
        RcDesc src{rc.shape, rc.id};
        sop.outs[i] = src;
    }

    for (const auto &in_nh : nh->inNodes())
    {
        putData(s, cg, in_nh);
    }

    for (const auto &out_nh : nh->outNodes())
    {
        putData(s, cg, out_nh);
    }

    s.m_ops.push_back(sop);
}

void printOp(const Op& op)
{
    std::cout << "printOp Op" << std::endl;
    std::cout << "printOp  Kernel" << std::endl;
    std::cout << "    " << op.k.name << std::endl;
    if (!op.k.tag.empty())
    {
        std::cout << op.k.tag << std::endl;
    }

    std::cout << "printOp  Args" << std::endl;
    size_t i_int = 0; size_t i_double = 0; size_t i_size = 0; size_t i_bool = 0;
    size_t i_scalar = 0; size_t i_point = 0; size_t i_mat = 0; size_t i_rect = 0;
    size_t i_objref = 0; size_t i_opaque = 0;
    for (const auto& kind : op.kind)
    {
        if(kind == (int)detail::ArgKind::GOBJREF)
        {
            const auto& rc = op.ins[i_objref]; i_objref++;
            std::cout << "printOp    rc.shape " << (int)rc.shape << ", rc.id " << rc.id  << std::endl;
        }
        else if(kind == (int)detail::ArgKind::OPAQUE)
        {
            switch ((detail::OpaqueKind)op.opaque_kind[i_opaque])
            {
            case detail::OpaqueKind::INT:
                std::cout << "printOp    int " << op.opaque_ints[i_int] << std::endl;
                i_int++;
                break;
            case detail::OpaqueKind::DOUBLE:
                std::cout << "printOp    double " << op.opaque_doubles[i_double] << std::endl;
                i_double++;
                break;
            case detail::OpaqueKind::CV_SIZE:
                std::cout << "printOp    cv::Size " << op.opaque_cvsizes[i_size].width << "x"
                          << op.opaque_cvsizes[i_size].height << std::endl;
                i_size++;
                break;
            case detail::OpaqueKind::BOOL:
                std::cout << "printOp    bool " << op.opaque_bools[i_bool] << std::endl;
                i_bool++;
                break;
            case detail::OpaqueKind::CV_SCALAR:
                std::cout << "printOp    cv::Scalar " << op.opaque_cvscalars[i_scalar][0] << " "
                          << op.opaque_cvscalars[i_scalar][1] << " "
                          << op.opaque_cvscalars[i_scalar][2] << " "
                          << op.opaque_cvscalars[i_scalar][3] << " "
                          << std::endl;
                i_scalar++;
                break;
            case detail::OpaqueKind::CV_POINT:
                std::cout << "printOp    cv::Point " << op.opaque_cvpoints[i_point].x << " "
                          << op.opaque_cvpoints[i_point].y << " "
                          << std::endl;
                i_point++;
                break;
            case detail::OpaqueKind::CV_MAT:
                std::cout << "printOp    cv::Mat " << op.opaque_cvmats[i_mat].rows << " "
                          << op.opaque_cvmats[i_mat].cols << " "
                          << op.opaque_cvmats[i_mat].type() << " "
                          << std::endl;
                i_mat++;
                break;
            case detail::OpaqueKind::CV_RECT:
                std::cout << "printOp     cv::Rect " << op.opaque_cvrects[i_rect].x << " "
                          << op.opaque_cvrects[i_rect].y << " "
                          << op.opaque_cvrects[i_rect].width << " "
                          << op.opaque_cvrects[i_rect].height << " "
                          << std::endl;
                i_rect++;
                break;
            default:
                std::cout << "printOp    OpaqueKind::UNSUPPORTED" << std::endl;
            }
        }
        else
        {
            util::throw_error(std::logic_error("Unexpected ArgKind: expected GOBJREF or OPAQUE"));
        }
        i_opaque++;
    }
    std::cout << "printOp  Outs" << std::endl;
    for (const auto& out : op.outs)
    {
        std::cout << "printOp    rc.shape " << (int)out.shape << ", rc.id " << out.id  << std::endl;
    }
}

void printData(const Data& data)
{
    std::cout << "printData Data" << std::endl;
    std::cout << "printData  rc.shape " << (int)data.rc.shape << ", rc.id = " << data.rc.id << std::endl;
    std::cout << "  " << data.meta << std::endl;
}

void printGSerialized(const GSerialized s)
{
    for (const auto& op : s.m_ops)
    {
        printOp(op);
    }

    for (const auto& data : s.m_datas)
    {
        printData(data);
    }
}
void dumpGSerializedOps(const GSerialized s, std::ofstream &ofs_ops)
{
    ofs_ops << s.m_ops;
}

void dumpGSerializedDatas(const GSerialized s, std::ofstream &ofs_data)
{
    ofs_data << s.m_datas;
}

void cleanGSerializedOps(GSerialized &s)
{

//    std::cout << "cleanGSerializedOps" <<  std::endl;
    for (uint i = 0; i < s.m_ops.size();  i++)
    {
        //Kernel k
        std::fill(s.m_ops[i].k.name.begin(), s.m_ops[i].k.name.end(), 0);
        s.m_ops[i].k.name.clear();
        std::fill(s.m_ops[i].k.tag.begin(), s.m_ops[i].k.tag.end(), 0);
        s.m_ops[i].k.tag.clear();

        //std::vector<int>   kind;
        std::fill(s.m_ops[i].kind.begin(), s.m_ops[i].kind.end(), 0);
        s.m_ops[i].kind.clear();

        //std::vector<int>   opaque_kind;
        std::fill(s.m_ops[i].opaque_kind.begin(), s.m_ops[i].opaque_kind.end(), 0);
        s.m_ops[i].opaque_kind.clear();

        RcDesc zero_desc; zero_desc.id = 0; zero_desc.shape = GShape::GOPAQUE;
        //std::vector<RcDesc> outs;
        std::fill(s.m_ops[i].outs.begin(), s.m_ops[i].outs.end(), zero_desc);
        s.m_ops[i].outs.clear();

        //std::vector<RcDesc> ins;
        std::fill(s.m_ops[i].ins.begin(), s.m_ops[i].ins.end(), zero_desc);
        s.m_ops[i].ins.clear();

        //opaque args
        //std::vector<int> opaque_ints;
        std::fill(s.m_ops[i].opaque_ints.begin(), s.m_ops[i].opaque_ints.end(), 0);
        s.m_ops[i].opaque_ints.clear();

        //std::vector<double> opaque_doubles;
        std::fill(s.m_ops[i].opaque_doubles.begin(), s.m_ops[i].opaque_doubles.end(), 0.0);
        s.m_ops[i].opaque_doubles.clear();

        //std::vector<cv::Size> opaque_cvsizes;
        std::fill(s.m_ops[i].opaque_cvsizes.begin(), s.m_ops[i].opaque_cvsizes.end(), cv::Size(0,0));
        s.m_ops[i].opaque_cvsizes.clear();

        //std::vector<bool> opaque_bools;
        std::fill(s.m_ops[i].opaque_bools.begin(), s.m_ops[i].opaque_bools.end(), false);
        s.m_ops[i].opaque_bools.clear();

        //std::vector<cv::Scalar> opaque_cvscalars;
        std::fill(s.m_ops[i].opaque_cvscalars.begin(), s.m_ops[i].opaque_cvscalars.end(), cv::Scalar(0,0,0,0));
        s.m_ops[i].opaque_cvscalars.clear();

        //std::vector<cv::Point> opaque_cvpoints;
        std::fill(s.m_ops[i].opaque_cvpoints.begin(), s.m_ops[i].opaque_cvpoints.end(), cv::Point(0,0));
        s.m_ops[i].opaque_cvpoints.clear();

        //std::vector<cv::Mat> opaque_cvmats;
        s.m_ops[i].opaque_cvmats.clear();

        //std::vector<cv::Rect> opaque_cvrects;
        std::fill(s.m_ops[i].opaque_cvrects.begin(), s.m_ops[i].opaque_cvrects.end(), cv::Rect(0, 0, 0, 0));
        s.m_ops[i].opaque_cvrects.clear();
    }
    s.m_ops.clear();
}

void readGSerializedOps(GSerialized &s, std::ifstream &ifs_ops)
{
    cleanGSerializedOps(s);
#if 0
    uint ops_size;
    ifs_ops.read((char*)&ops_size, sizeof(uint));
//    std::cout << "readGSerializedOps ops size " << ops_size <<  std::endl;

    s.m_ops.resize(ops_size);
    for (uint i = 0; i < ops_size;  i++)
    {
        std::cout << "readGSerializedOps kernel before " << s.m_ops[i].k.name.c_str() <<  std::endl;
        ifs_ops >> s.m_ops[i];
        std::cout << "readGSerializedOps kernel after " << s.m_ops[i].k.name.c_str() <<  std::endl;
    }
#else
    ifs_ops >> s.m_ops;
#endif
}

void cleanupGSerializedDatas(GSerialized &s)
{
    //std::cout << "cleanupGSerializedDatas" <<  std::endl;
    for (uint i = 0; i < s.m_datas.size(); i++)
    {
        //RcDesc rc;
        s.m_datas[i].rc.id = 0;
        s.m_datas[i].rc.shape = cv::GShape::GMAT;
        //GMetaArg meta;
        s.m_datas[i].meta = util::monostate();
    }
    s.m_datas.clear();
}


void readGSerializedDatas(GSerialized &s, std::ifstream &ifs_data)
{
#if 1
    cleanupGSerializedDatas(s);
    //s.m_datas.clear();
    ifs_data >> s.m_datas;
#else
    uint datas_size;
    ifs_data.read((char*)&datas_size, sizeof(uint));
    //std::cout << "readGSerializedDatas datas size " << datas_size <<  std::endl;
    for (uint i = 0; i < datas_size;  i++)
    {
#if 1
        ifs_data >> s.m_datas[i];
#else
        //RcDesc rc;
        RcDesc rc;
        ifs_data.read((char*)&rc, sizeof(RcDesc));
        //std::cout << "readGSerializedDatas RcDesc " << rc.id << " " << (int)rc.shape <<  std::endl;
        s.m_datas[i].rc.id = 0;
        s.m_datas[i].rc.shape = cv::GShape::GMAT;
        s.m_datas[i].rc = rc;

        //GMetaArg meta;
        switch (s.m_datas[i].rc.shape)
        {
        case cv::GShape::GMAT:
        {
            //std::cout << "readGSerializedDatas  GMAT " << std::endl;
            GMatDesc mat_desc;
            ifs_data.read((char*)&mat_desc.depth, sizeof(int));
            ifs_data.read((char*)&mat_desc.chan, sizeof(int));
            ifs_data.read((char*)&mat_desc.size, sizeof(cv::gapi::own::Size));
            int bool_val;
            ifs_data.read((char*)&bool_val, sizeof(int));
            mat_desc.planar = bool_val == 1 ? true : false;
//            std::cout << "readGSerializedDatas GMAT data " << mat_desc.depth << " "
//                      << mat_desc.chan << " "
//                      << mat_desc.size.width << " " << mat_desc.size.height << " "
//                      << mat_desc.planar << std::endl;
            uint dims_size;
            ifs_data.read((char*)&dims_size, sizeof(uint));
            //std::cout << "readGSerializedDatas  GMAT dims size " << dims_size << std::endl;
            std::vector<int> dims(dims_size);
            ifs_data.read((char*)dims.data(), dims_size * sizeof(int));
            mat_desc.dims = dims;
            for(uint j = 0; j < dims_size; j++)
            {
                std::cout << mat_desc.dims[j] << std::endl;
            }
            s.m_datas[i].meta = util::monostate();
            //s.m_datas[i].meta(std::move(mat_desc));
            s.m_datas[i].meta = GMatDesc(mat_desc.depth, mat_desc.chan, mat_desc.size, mat_desc.planar);
            break;
        }
        case cv::GShape::GSCALAR:
        {
            //std::cout << "readGSerializedDatas  GSCALAR " << std::endl;
            s.m_datas[i].meta = util::monostate();
            s.m_datas[i].meta = GScalarDesc();
            break;
        }
        case cv::GShape::GARRAY:
        {
            //std::cout << "readGSerializedDatas  GARRAY " << std::endl;
            s.m_datas[i].meta = util::monostate();
            s.m_datas[i].meta = GArrayDesc();
            break;
        }
        case cv::GShape::GOPAQUE:
        {
            //std::cout << "readGSerializedDatas  GOPAQUE " << std::endl;
            s.m_datas[i].meta = util::monostate();
            s.m_datas[i].meta = GOpaqueDesc();
            break;
        }
        default:
            std::cout << "readGSerializedDatas  unsupported" << std::endl;
        }
#endif
    }
#endif
}


void mkDataNode(ade::Graph& g, const Data& data)
{
    auto nh = g.createNode();
    GModel::Graph gm(g);

    gm.metadata(nh).set(NodeType{NodeType::DATA});

    HostCtor ctor{};
    // internal?
    auto storage = gimpl::Data::Storage::INTERNAL;
    gm.metadata(nh).set(gimpl::Data{data.rc.shape, data.rc.id, data.meta, ctor, storage});
}

void mkOpNode(ade::Graph& g, const Op& op)
{
    auto nh = g.createNode();
    GModel::Graph gm(g);

    gm.metadata(nh).set(NodeType{NodeType::OP});

    std::vector<gimpl::RcDesc> outs(op.outs.size());
    for (size_t i = 0; i < outs.size(); i++)
    {
        outs[i] = gimpl::RcDesc{op.outs[i].id, op.outs[i].shape, {}};
    }

    GArgs args(op.kind.size());
    size_t i_int = 0; size_t i_double = 0; size_t i_size = 0; size_t i_bool = 0;
    size_t i_scalar = 0; size_t i_point = 0; size_t i_mat = 0; size_t i_rect = 0;
    size_t i_objref = 0;
    for (size_t i = 0; i < args.size(); i++)
    {
        if(op.kind[i] == (int)detail::ArgKind::GOBJREF)
        {
            auto rc = op.ins[i_objref]; i_objref++;
            args[i] = GArg(gimpl::RcDesc{rc.id, rc.shape, {}});
        }
        else if(op.kind[i] == (int)detail::ArgKind::OPAQUE)
        {
            switch ((detail::OpaqueKind)op.opaque_kind[i])
            {
            case detail::OpaqueKind::INT:
            {
                auto opaque_int = op.opaque_ints[i_int]; i_int++;
                args[i] = GArg(opaque_int);
                std::cout << "mkOpNode    int " << args[i].get<int>() << std::endl;
                break;
            }
            case detail::OpaqueKind::DOUBLE:
            {
                auto opaque_double = op.opaque_doubles[i_double]; i_double++;
                args[i] = GArg(opaque_double);
                std::cout << "mkOpNode    double " << args[i].get<double>() << std::endl;
                break;
            }
            case detail::OpaqueKind::CV_SIZE:
            {
                auto opaque_cvsize = op.opaque_cvsizes[i_size]; i_size++;
                args[i] = GArg(opaque_cvsize);
                std::cout << "mkOpNode    cv::Size " << args[i].get<cv::Size>().width << "x"
                          << args[i].get<cv::Size>().height << std::endl;
                break;
            }
            case detail::OpaqueKind::BOOL:
            {
                bool opaque_bool = op.opaque_bools[i_bool]; i_bool++;
                args[i] = GArg(opaque_bool);
                std::cout << "mkOpNode    bool " << args[i].get<bool>() << std::endl;
                break;
            }
            case detail::OpaqueKind::CV_SCALAR:
            {
                auto opaque_cvscalar = op.opaque_cvscalars[i_scalar]; i_scalar++;
                args[i] = GArg(opaque_cvscalar);
                std::cout << "mkOpNode    cv::Scalar " << args[i].get<cv::Scalar>()[0] << " "
                          << args[i].get<cv::Scalar>()[1] << " "
                          << args[i].get<cv::Scalar>()[2] << " "
                          << args[i].get<cv::Scalar>()[3] << " "
                          << std::endl;
                break;
            }
            case detail::OpaqueKind::CV_POINT:
            {
                auto opaque_cvpoint = op.opaque_cvpoints[i_point]; i_point++;
                args[i] = GArg(opaque_cvpoint);
                std::cout << "mkOpNode    cv::Point " << args[i].get<cv::Point>().x << " "
                          << args[i].get<cv::Point>().y << " "
                          << std::endl;
                break;
            }
            case detail::OpaqueKind::CV_MAT:
            {
                auto opaque_cvmat = op.opaque_cvmats[i_mat]; i_mat++;
                args[i] = GArg(opaque_cvmat);
                std::cout << "mkOpNode    cv::Mat " << args[i].get<cv::Mat>().rows << " "
                          << args[i].get<cv::Mat>().cols << " "
                          << args[i].get<cv::Mat>().type() << " "
                          << std::endl;
                break;
            }
            case detail::OpaqueKind::CV_RECT:
            {
                auto opaque_cvrect = op.opaque_cvrects[i_rect]; i_rect++;
                args[i] = GArg(opaque_cvrect);
                std::cout << "mkOpNode     cv::Rect " << args[i].get<cv::Rect>().x << " "
                          << args[i].get<cv::Rect>().y << " "
                          << args[i].get<cv::Rect>().width << " "
                          << args[i].get<cv::Rect>().height << " "
                          << std::endl;
                break;
            }
            default:
                std::cout << "    OpaqueKind::UNSUPPORTED" << std::endl;
            }
        }
        else
        {
            util::throw_error(std::logic_error("Unexpected ArgKind: expected GOBJREF or OPAQUE"));
        }
    }

    //auto cpu_impl = cv::util::any_cast<cv::GCPUKernel>(op.k.Kernel.impl.opaque);
    //gm.metadata(nh).set(cv::gimpl::Unit{cpu_impl});
    gm.metadata(nh).set(gimpl::Op{cv::GKernel{op.k.name, op.k.tag, {},{}}, std::move(args), std::move(outs), {}});
}

std::vector<ade::NodeHandle> linkNodes(ade::Graph& g)
{
    GModel::Graph gm(g);
    GModel::ConstGraph cgm(g);

    using nodeMap = std::unordered_map<int, ade::NodeHandle>;
    std::unordered_map<cv::GShape, nodeMap> dataNodes;
    std::vector<ade::NodeHandle> nodes;

    for (const auto& nh : g.nodes())
    {
        if (cgm.metadata(nh).get<NodeType>().t == NodeType::DATA)
        {
            auto d = cgm.metadata(nh).get<gimpl::Data>();
            dataNodes[d.shape][d.rc] = nh;
        }
        nodes.push_back(nh);
    }

    for (const auto& nh : g.nodes())
    {
        if (cgm.metadata(nh).get<NodeType>().t == NodeType::OP)
        {
            const auto& op = cgm.metadata(nh).get<gimpl::Op>();

            for (const auto& in : ade::util::indexed(op.args))
            {
                const auto& arg = ade::util::value(in);
                if (arg.kind == detail::ArgKind::GOBJREF)
                {
                    const auto idx = ade::util::index(in);
                    const auto rc = arg.get<gimpl::RcDesc>();

                    const auto& inDataNode = dataNodes[rc.shape][rc.id];
                    const auto& e = g.link(inDataNode, nh);
                    gm.metadata(e).set(Input{idx});
                }
            }

            for (const auto& out : ade::util::indexed(op.outs))
            {
                const auto idx = ade::util::index(out);
                const auto rc = ade::util::value(out);

                const auto& outDataNode = dataNodes[rc.shape][rc.id];
                const auto& e = g.link(nh, outDataNode);
                gm.metadata(e).set(Output{idx});
            }
        }
    }
    return nodes;
}
//} // anonymous namespace

GSerialized serialize(const gimpl::GModel::ConstGraph& cg, const std::vector<ade::NodeHandle>& nodes)
{
    GSerialized s;
    for (auto &nh : nodes)
    {
        switch (cg.metadata(nh).get<NodeType>().t)
        {
        case NodeType::OP:   putOp  (s, cg, nh); break;
        case NodeType::DATA: putData(s, cg, nh); break;
        default: util::throw_error(std::logic_error("Unknown NodeType"));
        }
    }
    return s;
}

void deserialize(const serialization::GSerialized& s)
{
    printGSerialized(s);

    // FIXME: reuse code from GModelBuilder/GModel!
    // ObjectCounter?? (But seems we need existing mapping by shape+id)

    ade::Graph g;

    for (const auto& data : s.m_datas)
    {
        mkDataNode(g, data);
    }

    for (const auto& op : s.m_ops)
    {
        mkOpNode(g, op);
    }

    linkNodes(g);

//  FIXME:
//  Handle IslandModel!
//    std::shared_ptr<ade::Graph> ig;
//    GModel::Graph gm(g);
//    gm.metadata().set(gimpl::IslandModel{std::move(ig)});

    auto pass_ctx = ade::passes::PassContext{g};
    ade::passes::TopologicalSort{}(pass_ctx);
    gimpl::passes::dumpDotToFile(pass_ctx, "graph.dot");
}

//Graph dump operators
std::ostream& operator << (std::ostream& os, Kernel k)
{
    os << k.name;
    os << k.tag;
    return os;
}

std::ostream& operator << (std::ostream& os, std::string str)
{
    uint str_size = (uint)str.size();
    os.write((char*)&str_size, sizeof(uint));
    os.write((char*)str.c_str(), str_size * sizeof(char));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<int> ints)
{
    uint ints_size = (uint)ints.size();
    os.write((char*)&ints_size, sizeof(uint));
    os.write((char*)ints.data(), ints_size * sizeof(int));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<RcDesc> descs)
{
    uint descs_size = (uint)descs.size();
    os.write((char*)&descs_size, sizeof(uint));
    os.write((char*)descs.data(), descs_size * sizeof(RcDesc));
    return os;
}

std::ostream& operator << (std::ostream& os, RcDesc desc)
{
    os.write((char*)&desc,  sizeof(RcDesc));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<double> doubles)
{
    uint doubles_size = (uint)doubles.size();
    os.write((char*)&doubles_size, sizeof(uint));
    os.write((char*)doubles.data(), doubles_size * sizeof(double));
    return os;
}
std::ostream& operator << (std::ostream& os, std::vector<cv::Size> cvsizes)
{
    uint cvsizes_size = (uint)cvsizes.size();
    os.write((char*)&cvsizes_size, sizeof(uint));
    os.write((char*)cvsizes.data(), cvsizes_size * sizeof(cv::Size));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<bool> bools)
{
    uint bools_size = (uint)bools.size();
    os.write((char*)&bools_size, sizeof(uint));
    int bool_val;
    for(uint j = 0; j < bools_size; j++)
    {
        bool_val = bools[j] ? 1 : 0;
        os.write((char*)&bool_val, sizeof(int));
    }
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<cv::Scalar> cvscalars)
{
    uint cvscalars_size = (uint)cvscalars.size();
    os.write((char*)&cvscalars_size, sizeof(uint));
    os.write((char*)cvscalars.data(), cvscalars_size * sizeof(cv::Scalar));
    return os;
}
std::ostream& operator << (std::ostream& os, std::vector<cv::Point> cvpoints)
{
    uint cvpoints_size = (uint)cvpoints.size();
    os.write((char*)&cvpoints_size, sizeof(uint));
    os.write((char*)cvpoints.data(), cvpoints_size * sizeof(cv::Point));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<cv::Mat> cvmats)
{
    uint cvmats_size = (uint)cvmats.size();
    os.write((char*)&cvmats_size, sizeof(uint));
    for(uint j = 0; j < cvmats_size; j++)
    {
        size_t matSizeInBytes = cvmats[j].rows * cvmats[j].step[0];
        int mat_type = cvmats[j].type();
        os.write((char*)&cvmats[j].cols, sizeof(int));
        os.write((char*)&cvmats[j].rows, sizeof(int));
        os.write((char*)&mat_type, sizeof(int));
        os.write((char*)&cvmats[j].step[0], sizeof(int));
        if(matSizeInBytes!=0)
        {
            os.write((char*)cvmats[j].data, matSizeInBytes);
        }
    }
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<cv::Rect> cvrects)
{
    uint cvrects_size = (uint)cvrects.size();
    os.write((char*)&cvrects_size, sizeof(uint));
    os.write((char*)cvrects.data(), cvrects_size * sizeof(cv::Rect));
    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<Data> datas)
{
    uint datas_size = (uint)datas.size();
    os.write((char*)&datas_size, sizeof(uint));
    for (const auto& data : datas)
    {
        //Data
        os << data;
    }
    return os;
}

std::ostream& operator << (std::ostream& os, Data data)
{
    os << data.rc;

    //GMetaArg meta;
    switch (data.rc.shape)
    {
    case cv::GShape::GMAT:
    {
        GMatDesc mat_desc = util::get<GMatDesc>(data.meta);

        os.write((char*)&mat_desc.depth, sizeof(int));
        os.write((char*)&mat_desc.chan, sizeof(int));
        os.write((char*)&mat_desc.size, sizeof(cv::gapi::own::Size));
        int bool_val;
        bool_val = mat_desc.planar ? 1 : 0;
        os.write((char*)&bool_val, sizeof(int));

        uint dims_size = (uint)mat_desc.dims.size();
        os.write((char*)&dims_size, sizeof(uint));
        os.write((char*)mat_desc.dims.data(), dims_size * sizeof(int));
        break;
    }
    case cv::GShape::GSCALAR:
    {
        //std::cout << "dumpGSerializedDatas  GSCALAR " << std::endl;
        //GScalarDesc scalar_desc = util::get<GScalarDesc>(data.meta);
        break;
    }
    case cv::GShape::GARRAY:
    {   //std::cout << "dumpGSerializedDatas  GARRAY " << std::endl;
        //GArrayDesc array_desc = util::get<GArrayDesc>(data.meta);
        break;
    }
    case cv::GShape::GOPAQUE:
    {
        //std::cout << "dumpGSerializedDatas  GOPAQUE " << std::endl;
        //GOpaqueDesc opaque_desc = util::get<GOpaqueDesc>(data.meta);
        break;
    }
    default:
        //std::cout << "dumpGSerializedDatas  unsupported" << std::endl;
        break;
    }

    return os;
}

std::ostream& operator << (std::ostream& os, Op op)
{
    //Kernel k
    os << op.k;

    //std::vector<int>   kind;
    os << op.kind;

    //std::vector<int>   opaque_kind;
    os << op.opaque_kind;

    //std::vector<RcDesc> outs;
    os << op.outs;

    //std::vector<RcDesc> ins;
    os << op.ins;

    //opaque args
    //std::vector<int> opaque_ints;
    os << op.opaque_ints;

    //std::vector<double> opaque_doubles;
    os << op.opaque_doubles;

    //std::vector<cv::Size> opaque_cvsizes;
    os << op.opaque_cvsizes;

    //std::vector<bool> opaque_bools;
    os << op.opaque_bools;

    //std::vector<cv::Scalar> opaque_cvscalars;
    os << op.opaque_cvscalars;

    //std::vector<cv::Point> opaque_cvpoints;
    os << op.opaque_cvpoints;

    //std::vector<cv::Mat> opaque_cvmats;
    os << op.opaque_cvmats;

    //std::vector<cv::Rect> opaque_cvrects;
    os << op.opaque_cvrects;

    return os;
}

std::ostream& operator << (std::ostream& os, std::vector<Op> ops)
{
    uint ops_size = (uint)ops.size();
    os.write((char*)&ops_size, sizeof(uint));
    for (const auto& op : ops)
    {
        //Op
        os << op;
    }
    return os;
}

//Graph restore operators
std::istream& operator >> (std::istream& is, Kernel& k)
{
    is >> k.name;
    is >> k.tag;
    return is;
}

std::istream& operator >> (std::istream& is, std::string& str)
{
    uint str_size;
    is.read((char*)&str_size, sizeof(uint));
    str.resize(str_size);
    is.read((char*)str.c_str(), str_size * sizeof(char));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<int>& ints)
{
    uint ints_size;
    is.read((char*)&ints_size, sizeof(uint));
    ints.resize(ints_size);
    is.read((char*)ints.data(), ints_size * sizeof(int));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<RcDesc>& descs)
{
    uint descs_size;
    is.read((char*)&descs_size, sizeof(uint));
    descs.resize(descs_size);
    is.read((char*)descs.data(), descs_size * sizeof(RcDesc));
    return is;
}

std::istream& operator >> (std::istream& is, RcDesc& desc)
{
    is.read((char*)&desc,  sizeof(RcDesc));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<double>& doubles)
{
    uint doubles_size;
    is.read((char*)&doubles_size, sizeof(uint));
    doubles.resize(doubles_size);
    is.read((char*)doubles.data(), doubles_size * sizeof(double));
    return is;
}
std::istream& operator >> (std::istream& is, std::vector<cv::Size>& cvsizes)
{
    uint cvsizes_size;
    is.read((char*)&cvsizes_size, sizeof(uint));
    cvsizes.resize(cvsizes_size);
    is.read((char*)cvsizes.data(), cvsizes_size * sizeof(cv::Size));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<bool>& bools)
{
    uint bools_size;
    is.read((char*)&bools_size, sizeof(uint));
    bools.resize(bools_size);

    int bool_val;
    for(uint j = 0; j < bools_size; j++)
    {
        is.read((char*)&bool_val, sizeof(int));
        bools[j] = bool_val == 1 ? true: false;
    }
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<cv::Scalar>& cvscalars)
{
    uint cvscalars_size;
    is.read((char*)&cvscalars_size, sizeof(uint));
    cvscalars.resize(cvscalars_size);
    is.read((char*)cvscalars.data(), cvscalars_size * sizeof(cv::Scalar));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<cv::Point>& cvpoints)
{
    uint cvpoints_size;
    is.read((char*)&cvpoints_size, sizeof(uint));
    cvpoints.resize(cvpoints_size);
    is.read((char*)cvpoints.data(), cvpoints_size * sizeof(cv::Point));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<cv::Mat>& cvmats)
{
    uint cvmats_size = (uint)cvmats.size();
    is.read((char*)&cvmats_size, sizeof(uint));
    cvmats.resize(cvmats_size);
    for(uint j = 0; j < cvmats_size; j++)
    {
        int rows, cols, type, step;
        size_t matSizeInBytes;
        is.read((char*)&cols, sizeof(int));
        is.read((char*)&rows, sizeof(int));
        is.read((char*)&type, sizeof(int));
        is.read((char*)&step, sizeof(int));
        matSizeInBytes = rows*step;
        if(matSizeInBytes!=0)
        {
            void *mat_data = malloc(matSizeInBytes);
            is.read((char*)mat_data, matSizeInBytes);
            cv::Mat tmp_mat = cv::Mat(rows, cols, type, mat_data, step);
            tmp_mat.copyTo(cvmats[j]);
            free(mat_data);
        }
    }
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<cv::Rect>& cvrects)
{
    uint cvrects_size;
    is.read((char*)&cvrects_size, sizeof(uint));
    cvrects.resize(cvrects_size);
    is.read((char*)cvrects.data(), cvrects_size * sizeof(cv::Rect));
    return is;
}

std::istream& operator >> (std::istream& is, std::vector<Data>& datas)
{
    uint datas_size;
    is.read((char*)&datas_size, sizeof(uint));
    datas.resize(datas_size);
    for (uint i = 0; i < datas_size;  i++)
    {
        //Data
        is >> datas[i];
    }
    return is;
}

std::istream& operator >> (std::istream& is, Data& data)
{
    is >> data.rc;

    //GMetaArg meta;
    switch (data.rc.shape)
    {
    case cv::GShape::GMAT:
    {
        GMatDesc mat_desc;

        is.read((char*)&mat_desc.depth, sizeof(int));
        is.read((char*)&mat_desc.chan, sizeof(int));
        is.read((char*)&mat_desc.size, sizeof(cv::gapi::own::Size));

        int bool_val;
        is.read((char*)&bool_val, sizeof(int));
        mat_desc.planar = bool_val == 1 ? true : false;

        uint dims_size;
        is.read((char*)&dims_size, sizeof(uint));
        std::vector<int> dims(dims_size);
        is.read((char*)dims.data(), dims_size * sizeof(int));
        mat_desc.dims = dims;
        data.meta = GMatDesc(mat_desc.depth, mat_desc.chan, mat_desc.size, mat_desc.planar);
        break;
    }
    case cv::GShape::GSCALAR:
    {
        data.meta = GScalarDesc();
        break;
    }
    case cv::GShape::GARRAY:
    {
        data.meta = GArrayDesc();
        break;
    }
    case cv::GShape::GOPAQUE:
    {
        data.meta = GOpaqueDesc();
        break;
    }
    default:
        //std::cout << "dumpGSerializedDatas  unsupported" << std::endl;
        break;
    }

    return is;
}

std::istream& operator >> (std::istream& is, Op& op)
{
    //Kernel k
    is >> op.k;

    //std::vector<int>   kind;
    is >> op.kind;

    //std::vector<int>   opaque_kind;
    is >> op.opaque_kind;

    //std::vector<RcDesc> outs;
    is >> op.outs;

    //std::vector<RcDesc> ins;
    is >> op.ins;

    //opaque args
    //std::vector<int> opaque_ints;
    is >> op.opaque_ints;

    //std::vector<double> opaque_doubles;
    is >> op.opaque_doubles;

    //std::vector<cv::Size> opaque_cvsizes;
    is >> op.opaque_cvsizes;

    //std::vector<bool> opaque_bools;
    is >> op.opaque_bools;

    //std::vector<cv::Scalar> opaque_cvscalars;
    is >> op.opaque_cvscalars;

    //std::vector<cv::Point> opaque_cvpoints;
    is >> op.opaque_cvpoints;

    //std::vector<cv::Mat> opaque_cvmats;
    is >> op.opaque_cvmats;

    //std::vector<cv::Rect> opaque_cvrects;
    is >> op.opaque_cvrects;

    return is;
}

std::istream& operator >> (std::istream& is, std::vector<Op>& ops)
{
    uint ops_size;
    is.read((char*)&ops_size, sizeof(uint));
    ops.resize(ops_size);
    for (uint i = 0; i < ops_size;  i++)
    {
        //Op
        is >> ops[i];
    }
    return is;
}


} // namespace serialization
} // namespace gimpl
} // namespace cv
