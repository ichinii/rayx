#include "Tracer/CpuTracer.h"
#include "setupTests.h"

// TODO(rudi): shader tests

namespace RAYX {
namespace CPP_TRACER {
double r8_exp(double);
double r8_log(double);
double squaresDoubleRNG(uint64_t&);
Ray refrac2D(Ray, glm::dvec4, double, double);
Ray refrac(Ray, glm::dvec4, double);
glm::dvec4 normal_cartesian(glm::dvec4, double, double);
glm::dvec4 normal_cylindrical(glm::dvec4, double, double);
double wasteBox(double, double, double, double, double);
void RZPLineDensity(Ray r, glm::dvec4 normal, int IMAGE_TYPE, int RZP_TYPE,
                    int DERIVATION_METHOD, double zOffsetCenter, double risag,
                    double rosag, double rimer, double romer, double alpha,
                    double beta, double Ord, double WL, double& DX, double& DZ);
Ray rayMatrixMult(Ray, glm::dmat4);

}  // namespace CPP_TRACER
}  // namespace RAYX

using namespace RAYX;

TEST_F(TestSuite, testUniformRandom) {
    uint64_t ctr = 13;
    double old = 0;

    for (int i = 0; i < 100; i++) {
        double d = CPP_TRACER::squaresDoubleRNG(ctr);
        if (d == old) {
            RAYX_WARN << "repeating number in testUniformRandom! " << d;
        }
        if (d < 0.0 || d > 1.0) {
            RAYX_ERR << "random number out of range [0, 1]: " << d;
        }
        old = d;
    }
}

TEST_F(TestSuite, ExpTest) {
    std::vector<double> args = {10.0, 5.0, 2.0, 1.0, 0.5, 0.0001, 0.0};
    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_exp(x), exp(x));
        CHECK_EQ(CPP_TRACER::r8_exp(-x), exp(-x));
    }
}

// interestingly r8_log and log behave differently on input 0, namely 1 vs -inf.
TEST_F(TestSuite, LogTest) {
    std::vector<double> args = {10.0, 5.0, 2.0, 1.0, 0.5, 0.0001, 0.0000001};
    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_log(x), log(x));
    }
}

TEST_F(TestSuite, testRefrac2D) {
    std::vector<Ray> input = {
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.0001666666635802469, -0.017285764670739875,
                           0.99985057611723738),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(0.00016514977645243345, 0.012830838024391771, 0, 0),
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.00049999999722222276, -0.017285762731583675,
                           0.99985046502305308),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(-6.2949352042540596e-05, 0.038483898782123105, 0, 0),
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.0001666666635802469, -0.017619047234249029,
                           0.99984475864845179),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(-0.077169530850327184, 0.2686127340088395, 0, 0),
        },
        {
            .m_position = glm::dvec3(0.050470500672820856, 0.95514062789960541,
                                     -0.29182033770349547),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666666705, -0.016952478247434233,
                           0.99985617139734351),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(0.0021599283476277926, -0.050153240660177005, 0, 0),
        },
    };

    // the correct rays should only be altered in weight & direction.
    std::vector<Ray> correct = input;

    correct[0].m_weight = 1;
    correct[0].m_direction = glm::dvec3(
        -0.012664171360811521, 0.021648721107426414, 0.99968542634078494);

    correct[1].m_weight = 0;
    correct[1].m_direction = glm::dvec3(
        0.00049999999722222276, -0.017285762731583675, 0.99985046502305308);

    correct[2].m_weight = 0;
    correct[2].m_direction = glm::dvec3(
        0.0001666666635802469, -0.017619047234249029, 0.99984475864845179);

    correct[3].m_weight = 1;
    correct[3].m_direction = glm::dvec3(
        0.080765992839840872, 0.57052382524991363, 0.81730007905468893);

    CHECK_EQ(input.size(), correct.size());
    for (uint i = 0; i < input.size(); i++) {
        auto r = input[i];

        glm::dvec4 normal = glm::dvec4(r.m_position, 0);
        double az = r.m_stokes.x;
        double ax = r.m_stokes.y;

        auto out = CPP_TRACER::refrac2D(r, normal, az, ax);

        CHECK_EQ(out, correct[i]);
    }
}

TEST_F(TestSuite, testNormalCartesian) {
    struct InOutPair {
        glm::dvec4 in_normal;
        double in_slopeX;
        double in_slopeZ;

        glm::dvec4 out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(0, 1, 0, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(5.0465463027123736, 10470.451695989539,
                              -28.532199794465537, 0),
        },
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(-0.90019762973551742, 0.41198224566568298,
                              -0.14112000805986721, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(-9431.2371568647086, 4310.7269916467494,
                              -1449.3435640204684, 0),
        }};

    for (auto p : inouts) {
        auto out =
            CPP_TRACER::normal_cartesian(p.in_normal, p.in_slopeX, p.in_slopeZ);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testNormalCylindrical) {
    struct InOutPair {
        glm::dvec4 in_normal;
        double in_slopeX;
        double in_slopeZ;

        glm::dvec4 out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(0, 1, 0, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(5.0465463027115769, 10470.451695989539,
                              -28.532199794465537, 0),
        },
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(0.90019762973551742, 0.41198224566568292,
                              -0.14112000805986721, 0),
        },
        {.in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                 -28.532199794465537, 0),
         .in_slopeX = 2,
         .in_slopeZ = 3,
         .out = glm::dvec4(9431.2169472441783, 4310.7711493493844,
                           -1449.3437356459144, 0)}};

    for (auto p : inouts) {
        auto out = CPP_TRACER::normal_cylindrical(p.in_normal, p.in_slopeX,
                                                  p.in_slopeZ);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testRefrac) {
    std::vector<Ray> input = {
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666667084, -0.99558611855684065,
                           0.093851108341926226),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction = glm::dvec3(-1.6666664506172892e-05,
                                      -0.995586229182718, 0.093851118714515264),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0.0027574667592826954, 0.99999244446428082,
                                     -0.0027399619384214182),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666667084, -0.99558611855684065,
                           0.093851108341926226),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.99991341437509562, 0.013149667401360443,
                           -0.00049999997222215965),
            .m_energy = -0.038483898782123105,
        },
    };

    // the correct rays should only be altered in weight & direction.
    std::vector<Ray> correct = input;

    correct[0].m_weight = 1;
    correct[0].m_direction = glm::dvec3(
        -0.00049999991666667084, 0.99667709206767885, 0.08145258834192623);

    correct[1].m_weight = 1;
    correct[1].m_direction = glm::dvec3(
        -1.6666664506160695e-05, 0.9966772027014974, 0.081452598714515267);

    correct[2].m_weight = 1;
    correct[2].m_direction = glm::dvec3(
        0.0049947959329671825, 0.99709586573547515, 0.07599267429701162);

    correct[3].m_weight = 0;
    correct[3].m_direction = glm::dvec3(
        -0.99991341437509562, 0.013149667401360443, -0.00049999997222215965);

    CHECK_EQ(input.size(), correct.size());
    for (uint i = 0; i < input.size(); i++) {
        auto r = input[i];

        glm::dvec4 normal = glm::dvec4(r.m_position, 0);
        double a = r.m_energy;

        auto out = CPP_TRACER::refrac(r, normal, a);

        CHECK_EQ(out, correct[i]);
    }
}

TEST_F(TestSuite, testWasteBox) {
    struct InOutPair {
        double in_x;
        double in_z;
        double in_xLength;
        double in_zLength;
        double in_w;

        double out;
    };

    std::vector<InOutPair> inouts = {{
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 50,
                                         .in_zLength = 200,
                                         .in_w = 1,
                                         .out = 1,
                                     },
                                     {
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 5,
                                         .in_zLength = 20,
                                         .in_w = 1,
                                         .out = 0,
                                     },
                                     {
                                         .in_x = -1.6822205656320104,
                                         .in_z = 28.760233508097873,
                                         .in_xLength = 5,
                                         .in_zLength = 20,
                                         .in_w = 1,
                                         .out = 0,
                                     },
                                     {
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 50,
                                         .in_zLength = 200,
                                         .in_w = 0,
                                         .out = 0,
                                     }};

    for (auto p : inouts) {
        auto out = CPP_TRACER::wasteBox(p.in_x, p.in_z, p.in_xLength,
                                        p.in_zLength, p.in_w);

        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testRZPLineDensityDefaulParams) {
    struct InOutPair {
        Ray in_ray;
        glm::dvec4 in_normal;
        int in_imageType;
        int in_rzpType;
        int in_derivationMethod;
        double in_zOffsetCenter;
        double in_risag;
        double in_rosag;
        double in_rimer;
        double in_romer;
        double in_alpha;
        double in_beta;
        double in_Ord;
        double in_WL;

        double out_DX;
        double out_DZ;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_ray =
                {
                    .m_position =
                        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269),
                    .m_direction = glm::dvec3(0, 1, 0),
                },
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_imageType = 0,
            .in_rzpType = 0,
            .in_derivationMethod = 0,
            .in_zOffsetCenter = 0,
            .in_risag = 100,
            .in_rosag = 500,
            .in_rimer = 100,
            .in_romer = 500,
            .in_alpha = 0.017453292519943295,
            .in_beta = 0.017453292519943295,
            .in_Ord = -1,
            .in_WL = 1.239852e-05,

            .out_DX = 3103.9106911246749,
            .out_DZ = 5.0771666330055218,
        },
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6935030407867075, 0, 96.032777495754004),
                 .m_direction = glm::dvec3(0, 1, 0),
             },
         .in_normal = glm::dvec4(0, 1, 0, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1034.8685185321938,
         .out_DZ = -13.320120179862876},
        {.in_ray = {.m_position =
                        glm::dvec3(-5.047050067282087, 4.4859372100394515,
                                   29.182033770349552),
                    .m_direction =
                        glm::dvec3(0.05047050067282087, 0.95514062789960552,
                                   -0.29182033770349552)},
         .in_normal = glm::dvec4(0.05047050067282087, 0.95514062789960552,
                                 -0.29182033770349552, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 4045.0989844091882,
         .out_DZ = -174.2085626048659},
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6802365843267262, 1.3759250917712356,
                                16.445931214643075),
                 .m_direction =
                     glm::dvec3(0.016802365843267261, 0.98624074908228765,
                                -0.16445931214643075),
             },
         .in_normal = glm::dvec4(0.016802365843267261, 0.98624074908228765,
                                 -0.16445931214643075, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1418.1004208892475,
         .out_DZ = 253.09836635775162},
    };

    for (auto p : inouts) {
        double DX;
        double DZ;
        CPP_TRACER::RZPLineDensity(p.in_ray, p.in_normal, p.in_imageType,
                                   p.in_rzpType, p.in_derivationMethod,
                                   p.in_zOffsetCenter, p.in_risag, p.in_rosag,
                                   p.in_rimer, p.in_romer, p.in_alpha,
                                   p.in_beta, p.in_Ord, p.in_WL, DX, DZ);
        CHECK_EQ(DX, p.out_DX);
        CHECK_EQ(DZ, p.out_DZ);
    }
}

TEST_F(TestSuite, testRZPLineDensityAstigmatic) {
    struct InOutPair {
        Ray in_ray;
        glm::dvec4 in_normal;
        int in_imageType;
        int in_rzpType;
        int in_derivationMethod;
        double in_zOffsetCenter;
        double in_risag;
        double in_rosag;
        double in_rimer;
        double in_romer;
        double in_alpha;
        double in_beta;
        double in_Ord;
        double in_WL;

        double out_DX;
        double out_DZ;
    };

    std::vector<InOutPair> inouts = {

        {
            .in_ray =
                {
                    .m_position =
                        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269),
                    .m_direction = glm::dvec3(0, 1, 0),
                },
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_imageType = 0,
            .in_rzpType = 0,
            .in_derivationMethod = 0,
            .in_zOffsetCenter = 0,
            .in_risag = 100,
            .in_rosag = 500,
            .in_rimer = 100,
            .in_romer = 500,
            .in_alpha = 0.017453292519943295,
            .in_beta = 0.017453292519943295,
            .in_Ord = -1,
            .in_WL = 1.239852e-05,

            .out_DX = 3103.9106911246749,
            .out_DZ = 5.0771666330055218,
        },
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6935030407867075, 0, 96.032777495754004),
                 .m_direction = glm::dvec3(0, 1, 0),
             },
         .in_normal = glm::dvec4(0, 1, 0, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1034.8685185321938,
         .out_DZ = -13.320120179862876},
        {.in_ray = {.m_position =
                        glm::dvec3(-5.047050067282087, 4.4859372100394515,
                                   29.182033770349552),
                    .m_direction =
                        glm::dvec3(0.05047050067282087, 0.95514062789960552,
                                   -0.29182033770349552)},
         .in_normal = glm::dvec4(0.05047050067282087, 0.95514062789960552,
                                 -0.29182033770349552, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 4045.0989844091882,
         .out_DZ = -174.2085626048659},
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6802365843267262, 1.3759250917712356,
                                16.445931214643075),
                 .m_direction =
                     glm::dvec3(0.016802365843267261, 0.98624074908228765,
                                -0.16445931214643075),
             },
         .in_normal = glm::dvec4(0.016802365843267261, 0.98624074908228765,
                                 -0.16445931214643075, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1418.1004208892475,
         .out_DZ = 253.09836635775162},
    };

    for (auto p : inouts) {
        double DX;
        double DZ;
        CPP_TRACER::RZPLineDensity(p.in_ray, p.in_normal, p.in_imageType,
                                   p.in_rzpType, p.in_derivationMethod,
                                   p.in_zOffsetCenter, p.in_risag, p.in_rosag,
                                   p.in_rimer, p.in_romer, p.in_alpha,
                                   p.in_beta, p.in_Ord, p.in_WL, DX, DZ);
        CHECK_EQ(DX, p.out_DX);
        CHECK_EQ(DZ, p.out_DZ);
    }
}

TEST_F(TestSuite, testRayMatrixMult) {
    struct InOutPair {
        Ray in_ray;
        glm::dmat4 in_matrix;

        Ray out_ray;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(0, 0, 0),
                    .m_direction = glm::dvec3(0, 0, 0),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(13, 14, 15),
                    .m_direction = glm::dvec3(0, 0, 0),
                },
        },
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(1, 1, 0),
                    .m_direction = glm::dvec3(0, 1, 1),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(19, 22, 25),
                    .m_direction = glm::dvec3(14, 16, 18),
                },
        },
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(1, 2, 3),
                    .m_direction = glm::dvec3(4, 5, 6),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(51, 58, 65),
                    .m_direction = glm::dvec3(83, 98, 113),
                },
        },

    };

    for (auto p : inouts) {
        auto out_ray = CPP_TRACER::rayMatrixMult(p.in_ray, p.in_matrix);
        CHECK_EQ(out_ray, p.out_ray);
    }
}