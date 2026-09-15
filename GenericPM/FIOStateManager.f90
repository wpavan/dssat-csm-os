!=======================================================================
! FIOStateManager.f90
! Virginia Covert, Willingthon Pavan, Jose Mauricio Cunha Fernandes
! 04/15/2026
!
! Provides persistent state tracking for FIO soil property updates
!   without requiring additional subroutine parameters.
!-----------------------------------------------------------------------
! Licensing
! Copyright (c) 2026, University of Florida
! BSD-3-Clause. See the LICENSE file in the root folder for details.
! SPDX-License-Identifier: BSD-3-Clause
!-----------------------------------------------------------------------
!  Usage:
!    CALL SetFIOUpdateFlag()    ! Signal FIO needs update
!    CALL ClearFIOUpdateFlag()  ! Clear flag after update
!    IF (FIOUpdateNeeded()) THEN ... ! Check if update needed
!=======================================================================

MODULE FIOStateManager
      IMPLICIT NONE
      PRIVATE

      ! Private module state
      LOGICAL, SAVE :: FIO_UPDATE_FLAG = .FALSE.
      INTEGER, SAVE :: FIO_UPDATE_COUNT = 0
      
      ! Public procedures
      PUBLIC :: SetFIOUpdateFlag
      PUBLIC :: ClearFIOUpdateFlag
      PUBLIC :: FIOUpdateNeeded
      PUBLIC :: GetFIOUpdateCount
      PUBLIC :: ResetFIOUpdateCount

CONTAINS

!-----------------------------------------------------------------------
      SUBROUTINE SetFIOUpdateFlag()
      !  Signals that FIO soil properties should be updated
      !  Called from SOILDYN when SOM or tillage changes detected
      IMPLICIT NONE
      
      FIO_UPDATE_FLAG = .TRUE.
      FIO_UPDATE_COUNT = FIO_UPDATE_COUNT + 1
      
      END SUBROUTINE SetFIOUpdateFlag

!-----------------------------------------------------------------------
      SUBROUTINE ClearFIOUpdateFlag()
      !  Clears the update flag after FIO is updated
      !  Called from GDM or main program after PUT_FIO_SOILPROP()
      IMPLICIT NONE
      
      FIO_UPDATE_FLAG = .FALSE.
      
      END SUBROUTINE ClearFIOUpdateFlag

!-----------------------------------------------------------------------
      LOGICAL FUNCTION FIOUpdateNeeded()
      !  Returns .TRUE. if FIO soilprop update is needed
      !  Call this to check before updating FIO
      IMPLICIT NONE
      
      FIOUpdateNeeded = FIO_UPDATE_FLAG
      
      END FUNCTION FIOUpdateNeeded

!-----------------------------------------------------------------------
      INTEGER FUNCTION GetFIOUpdateCount()
      !  Returns count of FIO update signals (for diagnostics/timing)
      IMPLICIT NONE
      
      GetFIOUpdateCount = FIO_UPDATE_COUNT
      
      END FUNCTION GetFIOUpdateCount

!-----------------------------------------------------------------------
      SUBROUTINE ResetFIOUpdateCount()
      !  Resets update counter (useful for performance testing)
      IMPLICIT NONE
      
      FIO_UPDATE_COUNT = 0
      
      END SUBROUTINE ResetFIOUpdateCount

END MODULE FIOStateManager
!=======================================================================
