!=======================================================================
! fio_updater.for, Virginia Covert
! Subroutines which update the flexibleIO memory with 
!   GDM-relevant variables
!-----------------------------------------------------------------------
! REVISION HISTORY
! 04/14/2026 VC Written with PUT_FIO_WEATHER subroutine
!=======================================================================
SUBROUTINE PUT_FIO_WEATHER (WEATHER_arg, YEARDOY)
    ! Use only necessary external definitions
    USE ModuleDefs, ONLY: WeatherType, TS
    USE flexibleio

    IMPLICIT NONE

    ! Declare input-only arguments
    TYPE(WeatherType), INTENT(IN) :: WEATHER_arg
    TYPE(INTEGER)    , INTENT(IN) :: YEARDOY

    ! Declare local variables
    CHARACTER(len=2) :: HR_NUM
    INTEGER :: hour
    REAL :: retrieved_rhumhr

    ! Daily weather data.
    ! REAL: 
    ! CLOUDS, CO2, DAYL, DCO2, PAR, RAIN, RHUM, SNDN, SNUP,
    ! SRAD, TAMP, TA, TAV, TAVG, TDAY, TDEW, TGROAV, TGRODY,      
    ! TMAX, TMIN, TWILEN, VAPR, WINDRUN, WINDSP, VPDF, VPD_TRANSP,
    ! OZON7
    !
    ! LOGICAL:
    ! NOTDEW, NOWIND

    CALL fio%set("WTH", YEARDOY, "CLOUDS", WEATHER_arg % CLOUDS)
    CALL fio%set("WTH", YEARDOY, "CO2", WEATHER_arg % CO2)
    CALL fio%set("WTH", YEARDOY, "DAYL", WEATHER_arg % DAYL)
    CALL fio%set("WTH", YEARDOY, "DCO2", WEATHER_arg % DCO2)
    CALL fio%set("WTH", YEARDOY, "PAR", WEATHER_arg % PAR)
    CALL fio%set("WTH", YEARDOY, "RAIN", WEATHER_arg % RAIN)
    CALL fio%set("WTH", YEARDOY, "RHUM", WEATHER_arg % RHUM)
    CALL fio%set("WTH", YEARDOY, "SNDN", WEATHER_arg % SNDN)
    CALL fio%set("WTH", YEARDOY, "SNUP", WEATHER_arg % SNUP)
    CALL fio%set("WTH", YEARDOY, "SRAD", WEATHER_arg % SRAD)
    CALL fio%set("WTH", YEARDOY, "TAMP", WEATHER_arg % TAMP)
    CALL fio%set("WTH", YEARDOY, "TA", WEATHER_arg % TA)
    CALL fio%set("WTH", YEARDOY, "TAV", WEATHER_arg % TAV)
    CALL fio%set("WTH", YEARDOY, "TAVG", WEATHER_arg % TAVG)
    CALL fio%set("WTH", YEARDOY, "TDAY", WEATHER_arg % TDAY)
    CALL fio%set("WTH", YEARDOY, "TDEW", WEATHER_arg % TDEW)
    CALL fio%set("WTH", YEARDOY, "TGROAV", WEATHER_arg % TGROAV)
    CALL fio%set("WTH", YEARDOY, "TGRODY", WEATHER_arg % TGRODY)
    CALL fio%set("WTH", YEARDOY, "TMAX", WEATHER_arg % TMAX)
    CALL fio%set("WTH", YEARDOY, "TMIN", WEATHER_arg % TMIN)
    CALL fio%set("WTH", YEARDOY, "TWILEN", WEATHER_arg % TWILEN)
    CALL fio%set("WTH", YEARDOY, "VAPR", WEATHER_arg % VAPR)
    CALL fio%set("WTH", YEARDOY, "WINDRUN", WEATHER_arg % WINDRUN)
    CALL fio%set("WTH", YEARDOY, "WINDSP", WEATHER_arg % WINDSP)
    CALL fio%set("WTH", YEARDOY, "VPDF", WEATHER_arg % VPDF)
    CALL fio%set("WTH", YEARDOY, "VPD_TRANSP", WEATHER_arg % VPD_TRANSP)
    CALL fio%set("WTH", YEARDOY, "OZON7", WEATHER_arg % OZON7)

    ! Use the MERGE function, which can return a value based on a logical condition.
    CALL fio%set("WTH", YEARDOY, "NOTDEW", MERGE(1, 0, WEATHER_arg % NOTDEW))
    CALL fio%set("WTH", YEARDOY, "NOWIND", MERGE(1, 0, WEATHER_arg % NOWIND))

    ! Hourly weather data
    ! REAL, DIMENSION(TS): 
    ! AMTRH, AZZON, BETA, FRDIFP, FRDIFR, PARHR
    ! RADHR, RHUMHR, TAIRHR, TGRO, WINDHR

    ! Add all values by hour
    DO hour = 1, TS, 1
        WRITE(HR_NUM, '(I2.2)') hour
        CALL fio%set("WTH", YEARDOY, "AMTRH" // TRIM(HR_NUM), WEATHER_arg % AMTRH(hour))
        CALL fio%set("WTH", YEARDOY, "AZZON" // TRIM(HR_NUM), WEATHER_arg % AZZON(hour))
        CALL fio%set("WTH", YEARDOY, "BETA" // TRIM(HR_NUM), WEATHER_arg % BETA(hour))
        CALL fio%set("WTH", YEARDOY, "FRDIFP" // TRIM(HR_NUM), WEATHER_arg % FRDIFP(hour))
        CALL fio%set("WTH", YEARDOY, "FRDIFR" // TRIM(HR_NUM), WEATHER_arg % FRDIFR(hour))
        CALL fio%set("WTH", YEARDOY, "PARHR" // TRIM(HR_NUM), WEATHER_arg % PARHR(hour))
        CALL fio%set("WTH", YEARDOY, "RADHR" // TRIM(HR_NUM), WEATHER_arg % RADHR(hour))
        CALL fio%set("WTH", YEARDOY, "RHUMHR" // TRIM(HR_NUM), WEATHER_arg % RHUMHR(hour))
        CALL fio%set("WTH", YEARDOY, "TAIRHR" // TRIM(HR_NUM), WEATHER_arg % TAIRHR(hour))
        CALL fio%set("WTH", YEARDOY, "TGRO" // TRIM(HR_NUM), WEATHER_arg % TGRO(hour))
        CALL fio%set("WTH", YEARDOY, "WINDHR" // TRIM(HR_NUM), WEATHER_arg % WINDHR(hour))

        ! Debug to determine if values in fio get stored properly
        ! CALL fio%get("WTH", YEARDOY, "RHUMHR" // TRIM(HR_NUM), retrieved_rhumhr)
        ! WRITE(*,'(A, I0, A, I2.2, A, F0.4, A, F0.4)') &
        !     'DEBUG FIO RHUMHR: WTH/', YEARDOY, '/RHUMHR', hour, &
        !     ' | Input=', WEATHER_arg % RHUMHR(hour), &
        !     ' | Retrieved=', retrieved_rhumhr
    END DO
END SUBROUTINE

SUBROUTINE PUT_FIO_SOILPROP (SOILPROP_arg)
    USE ModuleDefs, ONLY: SoilType
    USE flexibleio

    IMPLICIT NONE

    TYPE(SoilType),    INTENT(IN) :: SOILPROP_arg
    
    INTEGER :: index

    ! Daily soil data.
    ! INTEGER: NLAYR
    ! LAYERED: SAT, DUL, LL, CLAY, SAND, SILT, STONES, BD

    CALL fio%set("SOIL", "NLAYR", SOILPROP_arg % NLAYR)

    ! Add all values by layer
    DO index = 1, SOILPROP_arg % NLAYR, 1
        CALL fio%set("SOIL", "SAT", SOILPROP_arg % SAT(index), index)
        CALL fio%set("SOIL", "DUL", SOILPROP_arg % DUL(index), index)
        CALL fio%set("SOIL", "LL", SOILPROP_arg % LL(index), index)
        CALL fio%set("SOIL", "BD", SOILPROP_arg % BD(index), index)
        CALL fio%set("SOIL", "CLAY", SOILPROP_arg % CLAY(index), index)
        CALL fio%set("SOIL", "SAND", SOILPROP_arg % SAND(index), index)
        CALL fio%set("SOIL", "SILT", SOILPROP_arg % SILT(index), index)
        CALL fio%set("SOIL", "STONES", SOILPROP_arg % STONES(index), index)
    END DO
END SUBROUTINE

SUBROUTINE PUT_FIO_SW (SW_arg)
    USE ModuleDefs, ONLY: NL
    USE flexibleio

    IMPLICIT NONE

    REAL, DIMENSION(NL) :: SW_arg
    INTEGER NLAYR
    INTEGER index

    CALL fio%get("SOIL", "NLAYR", NLAYR)

    IF (NLAYR .EQ. -99) THEN
        NLAYR = NL
    ENDIF

    DO index = 1, NLAYR, 1
        CALL fio%set("SOIL", "SW", SW_arg(index), index)
    END DO
END SUBROUTINE

SUBROUTINE PUT_FIO_CONTROL (CONTROL_arg)
    USE ModuleDefs, ONLY: ControlType
    USE flexibleio

    IMPLICIT NONE

    TYPE (ControlType) CONTROL_arg

    CALL fio%set("CONTROL", "YRDOY", CONTROL_arg % YRDOY)
    CALL fio%set("CONTROL", "YEARDOY", CONTROL_arg % YRDOY)
    CALL fio%set("CONTROL", "DAS", CONTROL_arg % DAS)
END SUBROUTINE

SUBROUTINE PUT_FIO_CROP (YEARDOY, ZSTAGE)
    USE flexibleio

    IMPLICIT NONE

    ! Declare input-only arguments
    TYPE(INTEGER)    , INTENT(IN) :: YEARDOY
    TYPE(REAL)       , INTENT(IN) :: ZSTAGE

    ! Declare local variables

    ! Make flexibleIO calls
    CALL fio%set("CROP", YEARDOY, "ZSTAGE", ZSTAGE)
END SUBROUTINE