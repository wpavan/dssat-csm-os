#ifndef COUPLING_H
#define COUPLING_H

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

enum class Direction {
    INPUT,      // Coupling point information coming from DSSAT to the GDM
    OUTPUT      // Coupling point information coming from GDM to DSSAT
};

enum class Basis {
    MASS,       // The coupling point value is in a mass basis
    AREA,       // The coupling point value is in an area basis
    NODE        // The coupling point value is in a node basis
};

enum class OrganType {
    LEAF,       // Coupling point refers to the leaves
    STEM,       // Coupling point refers to the stem/petiole
    ROOT,       // Coupling point refers to the roots
    ASM,        // Coupling point refers to the assimilate
    SEED        // Coupling point refers to the seeds/grain
};

enum class CouplingPointID{
    VALUE,      // Input - Total and constant value; special case for direct float values
    AREALF,     // Input - Area of leaves (one side) per unit ground area (cm2[leaf] / m2[ground])
    CLW,        // Input - Cumulative leaf growth (g[leaf]/m2)
    CSW,        // Input - Cumulative stem growth (g[stem]/m2)
    PCLMT,      // Input - Percent of total leaf mass (WTLF + senescence) destroyed (%)
    PCSTMD,     // Input - Observed cumulative percentage stem mass damage (%)
    PDLA,       // Input - Percent diseased leaf area (%)
    PLFAD,      // Input - Daily percent leaf area damage (%/d)
    PLFMD,      // Input - Daily percent leaf mass damage (%)
    PSTMD,      // Input - Daily percent stem mass damage (%)
    PVSTGD,     // Input - Percent V-stage damage (%)
    SLA,        // Input - Specific leaf area (cm2[leaf] / m2[ground])
    SLDOT,      // Input - Defoliation due to daily leaf senescence (g/m2/day)
    SSDOT,      // Input - Daily senescence of petioles (g / m2 / d)
    STMWT,      // Input - Dry mass of stem tissue, including C and N (g[stem] / m2[ground)
    TDLA,       // Input - Total diseased leaf area (cm2/m2)
    VSTGD,      // Input - Absolute daily V-stage damage (nodes/day)
    WLFDOT,     // Input - Leaf weight losses due to freezing (g[leaf]/m2-d)
    WSTMD,      // Input - Daily absolute stem damage (g/m2/day)
    WTLF,       // Input - Dry mass of leaf tissue including C and N (g[leaf] / m2[ground])
    TLFAD,      // Input - Total leaf area damage (cm2/cm2/d)
    TLFMD,      // Input - Total leaf mass damage (g/m2/day)
    VSTAGE,     // Input - Number of nodes on main stem of plant (nodes)
    WLIDOT,     // Output - Daily pest or freeze damage to leaf mass (g/m2/day)
    CLAI,       // Output - Cumulative leaf area index destroyed (m2/m2)
    CLFM,       // Output - Cumulative leaf mass destroyed  (g/m2)
    CSTEM,      // Output - Cumulative stem mass destroyed (g/m2)
    DISLA,      // Output - Diseased leaf area (cm2[leaf]/m2[ground]/d)
    DISLAP,     // Output - Percent diseased leaf area (%/d)
    LAIDOT,     // Output - Daily pest damage to leaf area index (m2/m2/d)    
    WSIDOT,     // Output - Daily pest damage to stem mass (g/m2/day)
    SDWT,       // Input - Seed weight (g/m2) NOTE: Check if this is an input or output.
    WSDD,       // Output - Daily weight of seed destroyed (g/m2/day)
    PSDD        // Output - Percent of seed mass destroyed (%/d)
};

struct CouplingData {
    private:
        // Constructor function
        CouplingData() {}

        CouplingData(const CouplingData&) = delete;
        CouplingData& operator=(const CouplingData&) = delete;

        static CouplingData* instance;

    public:
        float* AREALF = 0;
        float* CLW = 0;
        float* CSW = 0;
        float* PCLMT = 0;
        float* PCSTMD = 0;
        float* PDLA = 0;
        float* PLFAD = 0;
        float* PLFMD = 0;
        float* PSTMD = 0;
        float* PVSTGD = 0;
        float* SLA = 0;
        float* SLDOT = 0;
        float* SSDOT = 0;
        float* STMWT = 0;
        float* TDLA = 0;
        float* VSTGD = 0;
        float* WLFDOT = 0;
        float* WSTMD = 0;
        float* WTLF = 0;
        float* TLFAD = 0;
        float* TLFMD = 0;
        float* VSTAGE = 0;
        float* WLIDOT = 0;
        float* CLAI = 0;
        float* CLFM = 0;
        float* CSTEM = 0;
        float* DISLA = 0;
        float* DISLAP = 0;
        float* LAIDOT = 0;
        float* WSIDOT = 0;
        float* SDWT = 0;
        float* WSDD = 0;
        float* PSDD = 0;

        float AREALF_PREV = 0;
		float CLW_PREV = 0;
		float CSW_PREV = 0;
		float PCLMT_PREV = 0;
		float PCSTMD_PREV = 0;
		float PDLA_PREV = 0;
		float PLFAD_PREV = 0;
		float PLFMD_PREV = 0;
		float PSTMD_PREV = 0;
		float PVSTGD_PREV = 0;
		float SLA_PREV = 0;
		float SLDOT_PREV = 0;
		float SSDOT_PREV = 0;
		float STMWT_PREV = 0;
		float TDLA_PREV = 0;
		float VSTGD_PREV = 0;
		float WLFDOT_PREV = 0;
		float WSTMD_PREV = 0;
		float WTLF_PREV = 0;
		float TLFAD_PREV = 0;
		float TLFMD_PREV = 0;
		float VSTAGE_PREV = 0;
		float WLIDOT_PREV = 0;
		float CLAI_PREV = 0;
		float CLFM_PREV = 0;
		float CSTEM_PREV = 0;
		float DISLA_PREV = 0;
		float DISLAP_PREV = 0;
		float LAIDOT_PREV = 0;
		float WSIDOT_PREV = 0;
		float SDWT_PREV = 0;
		float WSDD_PREV = 0;
		float PSDD_PREV = 0;

        static CouplingData* getInstance() {
            if (instance == nullptr) {
                instance = new CouplingData();
            }
            return instance;
        }
       
        float* getCouplingValue(CouplingPointID cp);
        void setCouplingValue(CouplingPointID cp, float *value);
        void overwriteCouplingValue(CouplingPointID cp, float value);
        
        float getCouplingValuePrev(CouplingPointID cp);
        void updatePrevValues();   
};

struct CouplingPointTrait {
    Direction direction;
    OrganType organType;
    Basis basis;

    CouplingPointTrait(Direction d, OrganType o, Basis b) : direction(d), organType(o), basis(b) {}

    bool operator==(const CouplingPointTrait& other) const {
        return (direction == other.direction && organType == other.organType);
    }

    bool operator==(const Direction& dir) const {
        return (direction == dir);
    }

    bool operator==(const OrganType& type) const {
        return (organType == type);
    }

    bool operator==(const Basis& bas) const {
        return (basis == bas);
    }
};

class CouplingPoint {
    private:
        inline static std::unordered_map<CouplingPointID, CouplingPointTrait> traits;

    public:
        CouplingPoint() {
            initTraits();
        }

        static void initTraits() {
            if (traits.empty()) {
                traits.emplace(CouplingPointID::AREALF, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::CLW, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::CSW, CouplingPointTrait(Direction::INPUT,  OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::PCLMT, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::PCSTMD, CouplingPointTrait(Direction::INPUT,  OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::PDLA, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::PLFAD, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::PLFMD, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::PSTMD, CouplingPointTrait(Direction::INPUT,  OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::PVSTGD, CouplingPointTrait(Direction::INPUT,  OrganType::ASM,  Basis::NODE));
                traits.emplace(CouplingPointID::SLA, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::SLDOT, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::SSDOT, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::STMWT, CouplingPointTrait(Direction::INPUT,  OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::TDLA, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::VSTGD, CouplingPointTrait(Direction::INPUT,  OrganType::ASM,  Basis::NODE));
                traits.emplace(CouplingPointID::WLFDOT, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::WSTMD, CouplingPointTrait(Direction::INPUT,  OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::WTLF, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::TLFAD, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::TLFMD, CouplingPointTrait(Direction::INPUT,  OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::VSTAGE, CouplingPointTrait(Direction::INPUT,  OrganType::ASM,  Basis::NODE));
                traits.emplace(CouplingPointID::WLIDOT, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::CLAI, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::CLFM, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::MASS));
                traits.emplace(CouplingPointID::CSTEM, CouplingPointTrait(Direction::OUTPUT, OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::DISLA, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::DISLAP, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::LAIDOT, CouplingPointTrait(Direction::OUTPUT, OrganType::LEAF, Basis::AREA));
                traits.emplace(CouplingPointID::WSIDOT, CouplingPointTrait(Direction::OUTPUT, OrganType::STEM, Basis::MASS));
                traits.emplace(CouplingPointID::SDWT, CouplingPointTrait(Direction::INPUT,  OrganType::SEED, Basis::MASS));
                traits.emplace(CouplingPointID::WSDD, CouplingPointTrait(Direction::OUTPUT, OrganType::SEED, Basis::MASS));
                traits.emplace(CouplingPointID::PSDD, CouplingPointTrait(Direction::OUTPUT, OrganType::SEED, Basis::MASS));
            }
        }

        static CouplingPointTrait getTrait(CouplingPointID cpID) {
            initTraits();
            auto it = traits.find(cpID);
            if (it != traits.end()) {
                return it->second;
            }
            throw std::out_of_range("Invalid CouplingPointID in getTrait");
        }
};

CouplingPointID strToCPID(const std::string& str);
std::string cpIDToStr(CouplingPointID cp);

#endif // COUPLING_H