!=======================================================================
! fio_updater.for, Virginia Covert
! Subroutines which update the flexibleIO memory with 
!   GDM-relevant variables
!-----------------------------------------------------------------------
! REVISION HISTORY
! 04/14/2026 VC Written with PUT_FIO_WEATHER subroutine
!=======================================================================

SUBROUTINE PUT_FIO_WEATHER (WEATHER_arg, YEARDOY, debug_output)
    ! Use only necessary external definitions
    USE ModuleDefs, ONLY: WeatherType
    USE flexibleio

    IMPLICIT NONE

    ! Declare input-only arguments
    TYPE(WeatherType), INTENT(IN) :: WEATHER_arg
    TYPE(INTEGER)    , INTENT(IN) :: YEARDOY
    LOGICAL, INTENT(IN), OPTIONAL :: debug_output

    ! Local variable for debug output control
    LOGICAL :: debug

    ! Set debug output flag
    debug = .FALSE.
    IF (PRESENT(debug_output)) debug = debug_output

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
    IF (debug) WRITE(*,'(A,F10.4)') "  CLOUDS = ", WEATHER_arg % CLOUDS

    CALL fio%set("WTH", YEARDOY, "CO2", WEATHER_arg % CO2)
    IF (debug) WRITE(*,'(A,F10.4)') "  CO2 = ", WEATHER_arg % CO2

    CALL fio%set("WTH", YEARDOY, "DAYL", WEATHER_arg % DAYL)
    IF (debug) WRITE(*,'(A,F10.4)') "  DAYL = ", WEATHER_arg % DAYL

    CALL fio%set("WTH", YEARDOY, "DCO2", WEATHER_arg % DCO2)
    IF (debug) WRITE(*,'(A,F10.4)') "  DCO2 = ", WEATHER_arg % DCO2

    CALL fio%set("WTH", YEARDOY, "PAR", WEATHER_arg % PAR)
    IF (debug) WRITE(*,'(A,F10.4)') "  PAR = ", WEATHER_arg % PAR

    CALL fio%set("WTH", YEARDOY, "RAIN", WEATHER_arg % RAIN)
    IF (debug) WRITE(*,'(A,F10.4)') "  RAIN = ", WEATHER_arg % RAIN

    CALL fio%set("WTH", YEARDOY, "RHUM", WEATHER_arg % RHUM)
    IF (debug) WRITE(*,'(A,F10.4)') "  RHUM = ", WEATHER_arg % RHUM

    CALL fio%set("WTH", YEARDOY, "SNDN", WEATHER_arg % SNDN)
    IF (debug) WRITE(*,'(A,F10.4)') "  SNDN = ", WEATHER_arg % SNDN

    CALL fio%set("WTH", YEARDOY, "SNUP", WEATHER_arg % SNUP)
    IF (debug) WRITE(*,'(A,F10.4)') "  SNUP = ", WEATHER_arg % SNUP

    CALL fio%set("WTH", YEARDOY, "SRAD", WEATHER_arg % SRAD)
    IF (debug) WRITE(*,'(A,F10.4)') "  SRAD = ", WEATHER_arg % SRAD

    CALL fio%set("WTH", YEARDOY, "TAMP", WEATHER_arg % TAMP)
    IF (debug) WRITE(*,'(A,F10.4)') "  TAMP = ", WEATHER_arg % TAMP

    CALL fio%set("WTH", YEARDOY, "TA", WEATHER_arg % TA)
    IF (debug) WRITE(*,'(A,F10.4)') "  TA = ", WEATHER_arg % TA

    CALL fio%set("WTH", YEARDOY, "TAV", WEATHER_arg % TAV)
    IF (debug) WRITE(*,'(A,F10.4)') "  TAV = ", WEATHER_arg % TAV

    CALL fio%set("WTH", YEARDOY, "TAVG", WEATHER_arg % TAVG)
    IF (debug) WRITE(*,'(A,F10.4)') "  TAVG = ", WEATHER_arg % TAVG

    CALL fio%set("WTH", YEARDOY, "TDAY", WEATHER_arg % TDAY)
    IF (debug) WRITE(*,'(A,F10.4)') "  TDAY = ", WEATHER_arg % TDAY

    CALL fio%set("WTH", YEARDOY, "TDEW", WEATHER_arg % TDEW)
    IF (debug) WRITE(*,'(A,F10.4)') "  TDEW = ", WEATHER_arg % TDEW

    CALL fio%set("WTH", YEARDOY, "TGROAV", WEATHER_arg % TGROAV)
    IF (debug) WRITE(*,'(A,F10.4)') "  TGROAV = ", WEATHER_arg % TGROAV

    CALL fio%set("WTH", YEARDOY, "TGRODY", WEATHER_arg % TGRODY)
    IF (debug) WRITE(*,'(A,F10.4)') "  TGRODY = ", WEATHER_arg % TGRODY

    CALL fio%set("WTH", YEARDOY, "TMAX", WEATHER_arg % TMAX)
    IF (debug) WRITE(*,'(A,F10.4)') "  TMAX = ", WEATHER_arg % TMAX

    CALL fio%set("WTH", YEARDOY, "TMIN", WEATHER_arg % TMIN)
    IF (debug) WRITE(*,'(A,F10.4)') "  TMIN = ", WEATHER_arg % TMIN

    CALL fio%set("WTH", YEARDOY, "TWILEN", WEATHER_arg % TWILEN)
    IF (debug) WRITE(*,'(A,F10.4)') "  TWILEN = ", WEATHER_arg % TWILEN

    CALL fio%set("WTH", YEARDOY, "VAPR", WEATHER_arg % VAPR)
    IF (debug) WRITE(*,'(A,F10.4)') "  VAPR = ", WEATHER_arg % VAPR

    CALL fio%set("WTH", YEARDOY, "WINDRUN", WEATHER_arg % WINDRUN)
    IF (debug) WRITE(*,'(A,F10.4)') "  WINDRUN = ", WEATHER_arg % WINDRUN

    CALL fio%set("WTH", YEARDOY, "WINDSP", WEATHER_arg % WINDSP)
    IF (debug) WRITE(*,'(A,F10.4)') "  WINDSP = ", WEATHER_arg % WINDSP

    CALL fio%set("WTH", YEARDOY, "VPDF", WEATHER_arg % VPDF)
    IF (debug) WRITE(*,'(A,F10.4)') "  VPDF = ", WEATHER_arg % VPDF

    CALL fio%set("WTH", YEARDOY, "VPD_TRANSP", WEATHER_arg % VPD_TRANSP)
    IF (debug) WRITE(*,'(A,F10.4)') "  VPD_TRANSP = ", WEATHER_arg % VPD_TRANSP

    CALL fio%set("WTH", YEARDOY, "OZON7", WEATHER_arg % OZON7)
    IF (debug) WRITE(*,'(A,F10.4)') "  OZON7 = ", WEATHER_arg % OZON7

    ! Use the MERGE function, which can return a value based on a logical condition.
    CALL fio%set("WTH", YEARDOY, "NOTDEW", MERGE(1, 0, WEATHER_arg % NOTDEW))
    IF (debug) WRITE(*,'(A,I2)') "  NOTDEW = ", MERGE(1, 0, WEATHER_arg % NOTDEW)

    CALL fio%set("WTH", YEARDOY, "NOWIND", MERGE(1, 0, WEATHER_arg % NOWIND))
    IF (debug) WRITE(*,'(A,I2)') "  NOWIND = ", MERGE(1, 0, WEATHER_arg % NOWIND)



END SUBROUTINE

SUBROUTINE PUT_FIO_SOILPROP (SOILPROP_arg, debug_output)
    USE ModuleDefs, ONLY: SoilType
    USE flexibleio

    IMPLICIT NONE

    TYPE(SoilType),    INTENT(IN) :: SOILPROP_arg
    LOGICAL, INTENT(IN), OPTIONAL :: debug_output
    
    INTEGER :: index
    LOGICAL :: debug

    debug = .FALSE.
    IF (PRESENT(debug_output)) debug = debug_output
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
        IF (debug) WRITE(*,'(A, I0, A, F10.4, A, F10.4)') "Layer ", index, &
            ": SAT=", SOILPROP_arg % SAT(index), " LL=", SOILPROP_arg % LL(index)
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
    CALL fio%set("CONTROL", "YRDOY", CONTROL_arg % DAS)
END SUBROUTINE

