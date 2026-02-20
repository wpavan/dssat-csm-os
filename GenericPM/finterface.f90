!=======================================================================
! finterface.f90, Virginia Covert
! Fortran interface to C functions for GDM
!-----------------------------------------------------------------------
! REVISION HISTORY
! 08/18/2025 VC Written with READPESTGDM subroutine to isolate YAML reads
!               inside Generic Disease Module.
! 08/27/2025 VC Added subroutines bound to coupling init., rate, int.
!========================================================================

SUBROUTINE READPESTGDM(FILEPST, TRTNUM, FOUND)
    use, INTRINSIC :: iso_c_binding
    CHARACTER(len=*), intent(in) :: FILEPST
    CHARACTER(len=len_trim(FILEPST)+1, kind=c_char) :: FILEPST_C
    INTEGER TRTNUM
    INTEGER FOUND  

    interface
        subroutine readPstGdm(FILEPST, TRTNUM, FOUND)&
            bind(C, name='readPestYaml')
            
            import :: c_char, c_int
            CHARACTER(kind=c_char), dimension(*) :: FILEPST
            INTEGER(c_int) :: TRTNUM
            INTEGER(c_int) :: FOUND
        end subroutine readPstGdm
    end interface
    
    ! Copy the trimmed string and append a null terminator
    FILEPST_C = trim(FILEPST) // c_null_char

    call readPstGdm(FILEPST_C, TRTNUM, FOUND)
END SUBROUTINE READPESTGDM

SUBROUTINE INITGDM(YRDOY, YRPLT)
    USE, INTRINSIC :: iso_c_binding
    INTEGER YRDOY   ! Input - Current day of simulation (YYDDD)
    INTEGER YRPLT   ! Input - Planting date (YYDDD)

    INTERFACE
        SUBROUTINE couplingInit(YRDOY, YRPLT)&
            bind(C, name='couplingInit')
            INTEGER :: YRDOY
            INTEGER :: YRPLT
        END SUBROUTINE couplingInit
    END INTERFACE

    call couplingInit(YRDOY, YRPLT)
END SUBROUTINE INITGDM

SUBROUTINE RATEGDM(YRDOY,AREALF,CLW,CSW,PCLMT,PCSTMD,PDLA,PLFAD,PLFMD&
    ,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,TDLA,VSTGD,WLFDOT,WSTMD,WTLF&
    ,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI,CLFM,CSTEM,DISLA,DISLAP,LAIDOT&
    ,WSIDOT,SDWT,WSDD,PSDD,DAS,YRPLT)
    USE, INTRINSIC :: iso_c_binding
    INTEGER YRDOY   ! Input - Current day of simulation (YYDDD)
    REAL AREALF     ! Input - Area of leaves (one side) per unit ground area (cm2[leaf] / m2[ground])
    REAL CLW        ! Input - Cumulative leaf growth (g[leaf]/m2)
    REAL CSW        ! Input - Cumulative stem growth (g[stem]/m2)
    REAL PCLMT      ! Input - Percent of total leaf mass (WTLF + senescence) destroyed (%)
    REAL PCSTMD     ! Input - Observed cumulative percentage stem mass damage (%)
    REAL PDLA       ! Input - Percent diseased leaf area (%)
    REAL PLFAD      ! Input - Daily percent leaf area damage (%/d)
    REAL PLFMD      ! Input - Daily percent leaf mass damage (%)
    REAL PSTMD      ! Input - Daily percent stem mass damage (%)
    REAL PVSTGD     ! Input - Percent V-stage damage (%)
    REAL SLA        ! Input - Specific leaf area (cm2[leaf] / m2[ground])
    REAL SLDOT      ! Input - Defoliation due to daily leaf senescence (g/m2/day)
    REAL SSDOT      ! Input - Daily senescence of petioles (g / m2 / d)
    REAL STMWT      ! Input - Dry mass of stem tissue, including C and N (g[stem] / m2[ground)
    REAL TDLA       ! Input - Total diseased leaf area (cm2/m2)
    REAL VSTGD      ! Input - Absolute daily V-stage damage (nodes/day)
    REAL WLFDOT     ! Input - Leaf weight losses due to freezing (g[leaf]/m2-d)
    REAL WSTMD      ! Input - Daily absolute stem damage (g/m2/day)
    REAL WTLF       ! Input - Dry mass of leaf tissue including C and N (g[leaf] / m2[ground])
    REAL TLFAD      ! Input - Total leaf area damage (cm2/cm2/d)
    REAL TLFMD      ! Input - Total leaf mass damage (g/m2/day)
    REAL VSTAGE     ! Input - Number of nodes on main stem of plant (nodes)
    REAL WLIDOT     ! Output - Daily pest or freeze damage to leaf mass (g/m2/day)
    REAL CLAI       ! Output - Cumulative leaf area index destroyed (m2/m2)
    REAL CLFM       ! Output - Cumulative leaf mass destroyed  (g/m2)
    REAL CSTEM      ! Output - Cumulative stem mass destroyed (g/m2)
    REAL DISLA      ! Output - Diseased leaf area (cm2[leaf]/m2[ground]/d)
    REAL DISLAP     ! Output - Percent diseased leaf area (%/d)
    REAL LAIDOT     ! Output - Daily change in leaf area index due to pest damage (m2/m2/d)
    REAL WSIDOT     ! Output - Daily change in leaf area index due to pest damage (m2/m2/d)
    REAL SDWT
    REAL WSDD
    REAL PSDD
    INTEGER DAS     ! Input - Days After Simulation
    INTEGER YRPLT
    
    INTERFACE
        SUBROUTINE couplingRate(YRDOY,AREALF,CLW,CSW,PCLMT,PCSTMD,PDLA&
            ,PLFAD,PLFMD,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,TDLA,VSTGD&
            ,WLFDOT,WSTMD,WTLF,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI,CLFM&
            ,CSTEM,DISLA,DISLAP,LAIDOT,WSIDOT,SDWT,WSDD,PSDD,DAS,YRPLT&
            )bind(C, name='couplingRate')

            INTEGER :: YRDOY
            REAL :: AREALF
            REAL :: CLW
            REAL :: CSW
            REAL :: PCLMT
            REAL :: PCSTMD
            REAL :: PDLA
            REAL :: PLFAD
            REAL :: PLFMD
            REAL :: PSTMD
            REAL :: PVSTGD
            REAL :: SLA
            REAL :: SLDOT
            REAL :: SSDOT
            REAL :: STMWT
            REAL :: TDLA
            REAL :: VSTGD
            REAL :: WLFDOT
            REAL :: WSTMD
            REAL :: WTLF
            REAL :: TLFAD
            REAL :: TLFMD
            REAL :: VSTAGE
            REAL :: WLIDOT
            REAL :: CLAI
            REAL :: CLFM
            REAL :: CSTEM
            REAL :: DISLA
            REAL :: DISLAP
            REAL :: LAIDOT
            REAL :: WSIDOT
            REAL :: SDWT
            REAL :: WSDD
            REAL :: PSDD
            INTEGER :: DAS
            INTEGER :: YRPLT
        END SUBROUTINE couplingRate
    END INTERFACE

    CALL couplingRate(YRDOY,AREALF,CLW,CSW,PCLMT,PCSTMD,PDLA,PLFAD,&
        PLFMD,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,TDLA,VSTGD,WLFDOT,&
        WSTMD,WTLF,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI,CLFM,CSTEM,DISLA,&
        DISLAP,LAIDOT,WSIDOT,SDWT,WSDD,PSDD,DAS,YRPLT)
END SUBROUTINE RATEGDM 

SUBROUTINE INTEGRATIONGDM(YRDOY,AREALF,CLW,CSW,PCLMT,PCSTMD,PDLA,PLFAD,&
    PLFMD,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,TDLA,VSTGD,WLFDOT,WSTMD,&
    WTLF,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI,CLFM,CSTEM,DISLA,DISLAP,LAIDOT,&
    WSIDOT,SDWT,WSDD,PSDD,DAS)
    USE, INTRINSIC :: iso_c_binding
    INTEGER YRDOY   ! Input - Current day of simulation (YYDDD)
    REAL AREALF     ! Input - Area of leaves (one side) per unit ground area (cm2[leaf] / m2[ground])
    REAL CLW        ! Input - Cumulative leaf growth (g[leaf]/m2)
    REAL CSW        ! Input - Cumulative stem growth (g[stem]/m2)
    REAL PCLMT      ! Input - Percent of total leaf mass (WTLF + senescence) destroyed (%)
    REAL PCSTMD     ! Input - Observed cumulative percentage stem mass damage (%)
    REAL PDLA       ! Input - Percent diseased leaf area (%)
    REAL PLFAD      ! Input - Daily percent leaf area damage (%/d)
    REAL PLFMD      ! Input - Daily percent leaf mass damage (%)
    REAL PSTMD      ! Input - Daily percent stem mass damage (%)
    REAL PVSTGD     ! Input - Percent V-stage damage (%)
    REAL SLA        ! Input - Specific leaf area (cm2[leaf] / m2[ground])
    REAL SLDOT      ! Input - Defoliation due to daily leaf senescence (g/m2/day)
    REAL SSDOT      ! Input - Daily senescence of petioles (g / m2 / d)
    REAL STMWT      ! Input - Dry mass of stem tissue, including C and N (g[stem] / m2[ground)
    REAL TDLA       ! Input - Total diseased leaf area (cm2/m2)
    REAL VSTGD      ! Input - Absolute daily V-stage damage (nodes/day)
    REAL WLFDOT     ! Input - Leaf weight losses due to freezing (g[leaf]/m2-d)
    REAL WSTMD      ! Input - Daily absolute stem damage (g/m2/day)
    REAL WTLF       ! Input - Dry mass of leaf tissue including C and N (g[leaf] / m2[ground])
    REAL TLFAD      ! Input - Total leaf area damage (cm2/cm2/d)
    REAL TLFMD      ! Input - Total leaf mass damage (g/m2/day)
    REAL VSTAGE     ! Input - Number of nodes on main stem of plant (nodes)
    REAL WLIDOT     ! Output - Daily pest or freeze damage to leaf mass (g/m2/day)
    REAL CLAI       ! Output - Cumulative leaf area index destroyed (m2/m2)
    REAL CLFM       ! Output - Cumulative leaf mass destroyed  (g/m2)
    REAL CSTEM      ! Output - Cumulative stem mass destroyed (g/m2)
    REAL DISLA      ! Output - Diseased leaf area (cm2[leaf]/m2[ground]/d)
    REAL DISLAP     ! Output - Percent diseased leaf area (%/d)
    REAL LAIDOT     ! Output - Daily change in leaf area index due to pest damage (m2/m2/d)
    REAL WSIDOT     ! Output - Daily change in leaf area index due to pest damage (m2/m2/d)
    REAL SDWT
    REAL WSDD
    REAL PSDD
    INTEGER DAS     ! Input - Days After Simulation

    INTERFACE
        SUBROUTINE couplingIntegration(YRDOY,AREALF,CLW,CSW,PCLMT,&
            PCSTMD,PDLA,PLFAD,PLFMD,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,&
            TDLA,VSTGD,WLFDOT,WSTMD,WTLF,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI&
            ,CLFM,CSTEM,DISLA,DISLAP,LAIDOT,WSIDOT,SDWT,WSDD,PSDD,DAS)&
            bind(C, name="couplingIntegration")
            INTEGER :: YRDOY
            REAL :: AREALF
            REAL :: CLW
            REAL :: CSW
            REAL :: PCLMT
            REAL :: PCSTMD
            REAL :: PDLA
            REAL :: PLFAD
            REAL :: PLFMD
            REAL :: PSTMD
            REAL :: PVSTGD
            REAL :: SLA
            REAL :: SLDOT
            REAL :: SSDOT
            REAL :: STMWT
            REAL :: TDLA
            REAL :: VSTGD
            REAL :: WLFDOT
            REAL :: WSTMD
            REAL :: WTLF
            REAL :: TLFAD
            REAL :: TLFMD
            REAL :: VSTAGE
            REAL :: WLIDOT
            REAL :: CLAI
            REAL :: CLFM
            REAL :: CSTEM
            REAL :: DISLA
            REAL :: DISLAP
            REAL :: LAIDOT
            REAL :: WSIDOT
            REAL :: SDWT
            REAL :: WSDD
            REAL :: PSDD
            INTEGER :: DAS
        END SUBROUTINE couplingIntegration
    END INTERFACE 

    CALL couplingIntegration(YRDOY,AREALF,CLW,CSW,PCLMT,PCSTMD,PDLA,&
    PLFAD,PLFMD,PSTMD,PVSTGD,SLA,SLDOT,SSDOT,STMWT,TDLA,VSTGD,WLFDOT,&
    WSTMD,WTLF,TLFAD,TLFMD,VSTAGE,WLIDOT,CLAI,CLFM,CSTEM,DISLA,DISLAP,&
    LAIDOT,WSIDOT,SDWT,WSDD,PSDD,DAS)
END SUBROUTINE INTEGRATIONGDM

SUBROUTINE OUTPUTGDM(YRDOY)
    USE, INTRINSIC :: iso_c_binding
    INTEGER YRDOY   ! Input - Current day of simulation (YYDDD)

    INTERFACE
        SUBROUTINE couplingOutput(YRDOY)&
            bind(C, name='couplingOutput')
            INTEGER :: YRDOY
        END SUBROUTINE couplingOutput
    END INTERFACE

    call couplingOutput(YRDOY)
END SUBROUTINE OUTPUTGDM

SUBROUTINE LOGGINGGDM(YRDOY, YRSIM, SL1, SLL1, SSAT1, TMAX, TMIN, RAIN, SRAD, ZSTAGE)
    USE, INTRINSIC :: iso_c_binding
    INTEGER YRDOY
    INTEGER YRSIM
    REAL SL1, SLL1, SSAT1
    REAL TMAX, TMIN, RAIN, SRAD
    REAL ZSTAGE

    INTERFACE
        SUBROUTINE logger(YRDOY, YRSIM, SL1, SLL1, SSAT1, TMAX, TMIN, RAIN, SRAD, ZSTAGE)&
            bind(C, name='logger')

            INTEGER :: YRDOY
            INTEGER :: YRSIM
            REAL :: SL1
            REAL :: SLL1
            REAL :: SSAT1
            REAL :: TMAX
            REAL :: TMIN
            REAL :: RAIN
            REAL :: SRAD
            REAL :: ZSTAGE
        END SUBROUTINE logger
    END INTERFACE

    CALL logger(YRDOY, YRSIM, SL1, SLL1, SSAT1, TMAX, TMIN, RAIN, SRAD, ZSTAGE)
END SUBROUTINE LOGGINGGDM