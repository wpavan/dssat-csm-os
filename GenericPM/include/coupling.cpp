/**
 * @file coupling.cpp
 * 
 * @author Virginia L. Covert (virginiacovert454@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * 
 * @copyright Copyright (c) 2025-2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "coupling.h"

#include <stdexcept>

bool isConvertibleToFloat(const std::string& str) {
    try {
        size_t pos;
        std::stof(str, &pos); // Attempt conversion, store position of first unconverted char
        return pos == str.length(); // Check if the entire string was converted
    } catch (const std::invalid_argument& e) {
        return false; // Not a valid float format
    } catch (const std::out_of_range& e) {
        return false; // Value is out of float range
    }
}

// NOTE: Maybe create a dynamic way to add in new coupling points 
//       or aliases to coupling points 
CouplingPointID strToCPID(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty string cannot be converted to CouplingPointID");
    } else if (isConvertibleToFloat(str)) {
        return CouplingPointID::VALUE; // Special case for direct float values
    }
    if (str == "AREALF") return CouplingPointID::AREALF;
    if (str == "CLW") return CouplingPointID::CLW;
    if (str == "CSW") return CouplingPointID::CSW;
    if (str == "PCLMT") return CouplingPointID::PCLMT;
    if (str == "PCSTMD") return CouplingPointID::PCSTMD;

    // NOTE: Is this correct? Are there any other aliases we want to support? 
    if (str == "PDLA") return CouplingPointID::PDLA; 
    if (str == "PLFAD") return CouplingPointID::PLFAD;
    if (str == "PLFMD") return CouplingPointID::PLFMD;
    if (str == "PSTMD") return CouplingPointID::PSTMD;
    if (str == "PVSTGD") return CouplingPointID::PVSTGD;
    if (str == "SLA") return CouplingPointID::SLA;
    if (str == "SLDOT") return CouplingPointID::SLDOT;
    if (str == "SSDOT") return CouplingPointID::SSDOT;
    if (str == "STMWT") return CouplingPointID::STMWT;
    if (str == "TDLA") return CouplingPointID::TDLA;
    if (str == "VSTGD") return CouplingPointID::VSTGD;
    if (str == "WLFDOT") return CouplingPointID::WLFDOT;
    if (str == "WSTMD") return CouplingPointID::WSTMD;
    if (str == "WTLF") return CouplingPointID::WTLF;
    if (str == "TLFAD") return CouplingPointID::TLFAD;
    if (str == "TLFMD") return CouplingPointID::TLFMD;
    if (str == "VSTAGE") return CouplingPointID::VSTAGE;
    if (str == "WLIDOT") return CouplingPointID::WLIDOT;
    if (str == "CLAI") return CouplingPointID::CLAI;
    if (str == "CLFM") return CouplingPointID::CLFM;
    if (str == "CSTEM") return CouplingPointID::CSTEM;
    if (str == "DISLA") return CouplingPointID::DISLA;
    if (str == "DISLAP") return CouplingPointID::DISLAP;
    if (str == "LAIDOT") return CouplingPointID::LAIDOT;
    if (str == "WSIDOT") return CouplingPointID::WSIDOT;
    if (str == "SDWT") return CouplingPointID::SDWT;
    if (str == "WSDD") return CouplingPointID::WSDD;
    if (str == "PSDD") return CouplingPointID::PSDD;
    throw std::invalid_argument("Invalid CouplingPoint string: " + str);
}

std::string cpIDToStr(CouplingPointID cp) {
    switch (cp) {
        case CouplingPointID::AREALF: return "AREALF";
        case CouplingPointID::CLW: return "CLW";
        case CouplingPointID::CSW: return "CSW";
        case CouplingPointID::PCLMT: return "PCLMT";
        case CouplingPointID::PCSTMD: return "PCSTMD";
        case CouplingPointID::PDLA: return "PDLA";
        case CouplingPointID::PLFAD: return "PLFAD";
        case CouplingPointID::PLFMD: return "PLFMD";
        case CouplingPointID::PSTMD: return "PSTMD";
        case CouplingPointID::PVSTGD: return "PVSTGD";
        case CouplingPointID::SLA: return "SLA";
        case CouplingPointID::SLDOT: return "SLDOT";
        case CouplingPointID::SSDOT: return "SSDOT";
        case CouplingPointID::STMWT: return "STMWT";
        case CouplingPointID::TDLA: return "TDLA";
        case CouplingPointID::VSTGD: return "VSTGD";
        case CouplingPointID::WLFDOT: return "WLFDOT";
        case CouplingPointID::WSTMD: return "WSTMD";
        case CouplingPointID::WTLF: return "WTLF";
        case CouplingPointID::TLFAD: return "TLFAD";
        case CouplingPointID::TLFMD: return "TLFMD";
        case CouplingPointID::VSTAGE: return "VSTAGE";
        case CouplingPointID::WLIDOT: return "WLIDOT";
        case CouplingPointID::CLAI: return "CLAI";
        case CouplingPointID::CLFM: return "CLFM";
        case CouplingPointID::CSTEM: return "CSTEM";
        case CouplingPointID::DISLA: return "DISLA";
        case CouplingPointID::DISLAP: return "DISLAP";
        case CouplingPointID::LAIDOT: return "LAIDOT";
        case CouplingPointID::WSIDOT: return "WSIDOT";
        case CouplingPointID::SDWT: return "SDWT";
        case CouplingPointID::WSDD: return "WSDD";
        case CouplingPointID::PSDD: return "PSDD";
        case CouplingPointID::VALUE: return "VALUE";
        default:
            throw std::invalid_argument("Invalid CouplingPoint: " + std::to_string(static_cast<int>(cp)));
    }
}

CouplingData* CouplingData::instance = nullptr;

float* CouplingData::getCouplingValue(CouplingPointID cp) {
    switch (cp) {
        case CouplingPointID::AREALF: return AREALF;
        case CouplingPointID::CLW: return CLW;
        case CouplingPointID::CSW: return CSW;
        case CouplingPointID::PCLMT: return PCLMT;
        case CouplingPointID::PCSTMD: return PCSTMD;
        case CouplingPointID::PDLA: return PDLA;
        case CouplingPointID::PLFAD: return PLFAD;
        case CouplingPointID::PLFMD: return PLFMD;
        case CouplingPointID::PSTMD: return PSTMD;
        case CouplingPointID::PVSTGD: return PVSTGD;
        case CouplingPointID::SLA: return SLA;
        case CouplingPointID::SLDOT: return SLDOT;
        case CouplingPointID::SSDOT: return SSDOT;
        case CouplingPointID::STMWT: return STMWT;
        case CouplingPointID::TDLA: return TDLA;
        case CouplingPointID::VSTGD: return VSTGD;
        case CouplingPointID::WLFDOT: return WLFDOT;
        case CouplingPointID::WSTMD: return WSTMD;
        case CouplingPointID::WTLF: return WTLF;
        case CouplingPointID::TLFAD: return TLFAD;
        case CouplingPointID::TLFMD: return TLFMD;
        case CouplingPointID::VSTAGE: return VSTAGE;
        case CouplingPointID::WLIDOT: return WLIDOT;
        case CouplingPointID::CLAI: return CLAI;
        case CouplingPointID::CLFM: return CLFM;
        case CouplingPointID::CSTEM: return CSTEM;
        case CouplingPointID::DISLA: return DISLA;
        case CouplingPointID::DISLAP: return DISLAP;
        case CouplingPointID::LAIDOT: return LAIDOT;
        case CouplingPointID::WSIDOT: return WSIDOT;
        case CouplingPointID::SDWT: return SDWT;
        case CouplingPointID::WSDD: return WSDD;
        case CouplingPointID::PSDD: return PSDD;
        default:
            throw std::invalid_argument("Invalid CouplingPoint: " + std::to_string(static_cast<int>(cp)));
    }
}

void CouplingData::setCouplingValue(CouplingPointID cp, float *value) {
    // NOTE: Add any protections on the inputs here if necessary.
    switch (cp) {
        case CouplingPointID::AREALF: AREALF = value; break;
        case CouplingPointID::CLW: CLW = value; break;
        case CouplingPointID::CSW: CSW = value; break;
        case CouplingPointID::PCLMT: PCLMT = value; break;
        case CouplingPointID::PCSTMD: PCSTMD = value; break;
        case CouplingPointID::PDLA: PDLA = value; break;
        case CouplingPointID::PLFAD: PLFAD = value; break;
        case CouplingPointID::PLFMD: PLFMD = value; break;
        case CouplingPointID::PSTMD: PSTMD = value; break;
        case CouplingPointID::PVSTGD: PVSTGD = value; break;
        case CouplingPointID::SLA: SLA = value; break;
        case CouplingPointID::SLDOT: SLDOT = value; break;
        case CouplingPointID::SSDOT: SSDOT = value; break;
        case CouplingPointID::STMWT: STMWT = value; break;
        case CouplingPointID::TDLA: TDLA = value; break;
        case CouplingPointID::VSTGD: VSTGD = value; break;
        case CouplingPointID::WLFDOT: WLFDOT = value; break;
        case CouplingPointID::WSTMD: WSTMD = value; break;
        case CouplingPointID::WTLF: WTLF = value; break;
        case CouplingPointID::TLFAD: TLFAD = value; break;
        case CouplingPointID::TLFMD: TLFMD = value; break;
        case CouplingPointID::VSTAGE: VSTAGE = value; break;
        case CouplingPointID::WLIDOT: WLIDOT = value; break;
        case CouplingPointID::CLAI: CLAI = value; break;
        case CouplingPointID::CLFM: CLFM = value; break;
        case CouplingPointID::CSTEM: CSTEM = value; break;
        case CouplingPointID::DISLA: DISLA = value; break;
        case CouplingPointID::DISLAP: DISLAP = value; break;
        case CouplingPointID::LAIDOT: LAIDOT = value; break;
        case CouplingPointID::WSIDOT: WSIDOT = value; break;
        case CouplingPointID::SDWT: SDWT = value; break;
        case CouplingPointID::WSDD: WSDD = value; break;
        case CouplingPointID::PSDD: PSDD = value; break;
        default:
            throw std::invalid_argument("Invalid CouplingPoint: " + std::to_string(static_cast<int>(cp)));
    }
}

void CouplingData::overwriteCouplingValue(CouplingPointID cp, float value) {
    // NOTE: Add any protections on the inputs here if necessary.
    switch (cp) {
        case CouplingPointID::AREALF: *AREALF = value; break;
        case CouplingPointID::CLW: *CLW = value; break;
        case CouplingPointID::CSW: *CSW = value; break;
        case CouplingPointID::PCLMT: *PCLMT = value; break;
        case CouplingPointID::PCSTMD: *PCSTMD = value; break;
        case CouplingPointID::PDLA: *PDLA = value; break;
        case CouplingPointID::PLFAD: *PLFAD = value; break;
        case CouplingPointID::PLFMD: *PLFMD = value; break;
        case CouplingPointID::PSTMD: *PSTMD = value; break;
        case CouplingPointID::PVSTGD: *PVSTGD = value; break;
        case CouplingPointID::SLA: *SLA = value; break;
        case CouplingPointID::SLDOT: *SLDOT = value; break;
        case CouplingPointID::SSDOT: *SSDOT = value; break;
        case CouplingPointID::STMWT: *STMWT = value; break;
        case CouplingPointID::TDLA: *TDLA = value; break;
        case CouplingPointID::VSTGD: *VSTGD = value; break;
        case CouplingPointID::WLFDOT: *WLFDOT = value; break;
        case CouplingPointID::WSTMD: *WSTMD = value; break;
        case CouplingPointID::WTLF: *WTLF = value; break;
        case CouplingPointID::TLFAD: *TLFAD = value; break;
        case CouplingPointID::TLFMD: *TLFMD = value; break;
        case CouplingPointID::VSTAGE: *VSTAGE = value; break;
        case CouplingPointID::WLIDOT: *WLIDOT = value; break;
        case CouplingPointID::CLAI: *CLAI = value; break;
        case CouplingPointID::CLFM: *CLFM = value; break;
        case CouplingPointID::CSTEM: *CSTEM = value; break;
        case CouplingPointID::DISLA: *DISLA = value; break;
        case CouplingPointID::DISLAP: *DISLAP = value; break;
        case CouplingPointID::LAIDOT: *LAIDOT = value; break;
        case CouplingPointID::WSIDOT: *WSIDOT = value; break;
        case CouplingPointID::SDWT: *SDWT = value; break;
        case CouplingPointID::WSDD: *WSDD = value; break;
        case CouplingPointID::PSDD: *PSDD = value; break;
        default:
            throw std::invalid_argument("Invalid CouplingPoint: " + std::to_string(static_cast<int>(cp)));
    }
}

float CouplingData::getCouplingValuePrev(CouplingPointID cp) {
    switch (cp) {
        case CouplingPointID::AREALF: return AREALF_PREV;
        case CouplingPointID::CLW: return CLW_PREV;
        case CouplingPointID::CSW: return CSW_PREV;
        case CouplingPointID::PCLMT: return PCLMT_PREV;
        case CouplingPointID::PCSTMD: return PCSTMD_PREV;
        case CouplingPointID::PDLA: return PDLA_PREV;
        case CouplingPointID::PLFAD: return PLFAD_PREV;
        case CouplingPointID::PLFMD: return PLFMD_PREV;
        case CouplingPointID::PSTMD: return PSTMD_PREV;
        case CouplingPointID::PVSTGD: return PVSTGD_PREV;
        case CouplingPointID::SLA: return SLA_PREV;
        case CouplingPointID::SLDOT: return SLDOT_PREV;
        case CouplingPointID::SSDOT: return SSDOT_PREV;
        case CouplingPointID::STMWT: return STMWT_PREV;
        case CouplingPointID::TDLA: return TDLA_PREV;
        case CouplingPointID::VSTGD: return VSTGD_PREV;
        case CouplingPointID::WLFDOT: return WLFDOT_PREV;
        case CouplingPointID::WSTMD: return WSTMD_PREV;
        case CouplingPointID::WTLF: return WTLF_PREV;
        case CouplingPointID::TLFAD: return TLFAD_PREV;
        case CouplingPointID::TLFMD: return TLFMD_PREV;
        case CouplingPointID::VSTAGE: return VSTAGE_PREV;
        case CouplingPointID::WLIDOT: return WLIDOT_PREV;
        case CouplingPointID::CLAI: return CLAI_PREV;
        case CouplingPointID::CLFM: return CLFM_PREV;
        case CouplingPointID::CSTEM: return CSTEM_PREV;
        case CouplingPointID::DISLA: return DISLA_PREV;
        case CouplingPointID::DISLAP: return DISLAP_PREV;
        case CouplingPointID::LAIDOT: return LAIDOT_PREV;
        case CouplingPointID::WSIDOT: return WSIDOT_PREV;
        case CouplingPointID::SDWT: return SDWT_PREV;
        case CouplingPointID::WSDD: return WSDD_PREV;
        case CouplingPointID::PSDD: return PSDD_PREV;
        default:
            throw std::invalid_argument("Invalid CouplingPoint: " + std::to_string(static_cast<int>(cp)));
    }
}

void CouplingData::updatePrevValues() {
    AREALF_PREV = *AREALF;
    CLW_PREV = *CLW;
    CSW_PREV = *CSW;
    PCLMT_PREV = *PCLMT;
    PCSTMD_PREV = *PCSTMD;
    PDLA_PREV = *PDLA;
    PLFAD_PREV = *PLFAD;
    PLFMD_PREV = *PLFMD;
    PSTMD_PREV = *PSTMD;
    PVSTGD_PREV = *PVSTGD;
    SLA_PREV = *SLA;
    SLDOT_PREV = *SLDOT;
    SSDOT_PREV = *SSDOT;
    STMWT_PREV = *STMWT;
    TDLA_PREV = *TDLA;
    VSTGD_PREV = *VSTGD;
    WLFDOT_PREV = *WLFDOT;
    WSTMD_PREV = *WSTMD;
    WTLF_PREV = *WTLF;
    TLFAD_PREV = *TLFAD;
    TLFMD_PREV = *TLFMD;
    VSTAGE_PREV = *VSTAGE;
    WLIDOT_PREV = *WLIDOT;
    CLAI_PREV = *CLAI;
    CLFM_PREV = *CLFM;
    CSTEM_PREV = *CSTEM;
    DISLA_PREV = *DISLA;
    DISLAP_PREV = *DISLAP;
    LAIDOT_PREV = *LAIDOT;
    WSIDOT_PREV = *WSIDOT;
    SDWT_PREV = *SDWT;
    WSDD_PREV = *WSDD;
    PSDD_PREV = *PSDD;
}
