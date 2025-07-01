USE [master]
GO

CREATE DATABASE [User]
ON PRIMARY
( NAME = N'User', FILENAME = N'C:\Program Files\Microsoft SQL Server\MSSQL16.SQLEXPRESS\MSSQL\DATA\User.mdf',
SIZE = 102400KB, MAXSIZE = UNLIMITED, FILEGROWTH = 10240KB )
LOG ON
( NAME = N'User_Log', FILENAME = N'C:\Program Files\Microsoft SQL Server\MSSQL16.SQLEXPRESS\MSSQL\DATA\User_Log.ldf',
SIZE = 102400KB, MAXSIZE = 2048GB, FILEGROWTH = 10240KB )
GO

ALTER DATABASE [User] SET COMPATIBILITY_LEVEL = 100
GO


USE [User]
GO

--테이블 정의

CREATE SEQUENCE AccountNumberSeq
    AS BIGINT
    START WITH 1000000
    INCREMENT BY 1;

CREATE TABLE [dbo].[Account]
	(
	[AccountNumber] [bigint] NOT NULL DEFAULT NEXT VALUE FOR AccountNumberSeq, -- SEQUENCE 
	[AccountUID] [varchar](55) UNIQUE NOT NULL,
	[CreateDate] [datetime] NOT NULL,
	[LastSignIndDate] [datetime],
	[IsActive] [tinyint] NOT NULL, -- 현재 접속종료:0, 접속중 : 1 
	CONSTRAINT [PK_Account_AccountNumber] PRIMARY KEY CLUSTERED ([AccountNumber] ASC)
	WITH (
		PAD_INDEX = ON, --상위 레벨 인덱스 페이지에도 여유 공간을 확보하도록 설정
		STATISTICS_NORECOMPUTE = OFF,-- 자동으로 통계정보를 업데이트 하도록 설정
		IGNORE_DUP_KEY = OFF,--중복 키 입력시 오류발생
		ALLOW_ROW_LOCKS = ON,-- 행단위 잠금 허용 -> 동시성 증가
		ALLOW_PAGE_LOCKS = ON,-- 페이지 단위 잠금 허용 -> 많은 데이터를 동시에 업데이트 할 때 성능 향상 가능
		FILLFACTOR = 90,-- 인덱스 페이지를 90%만채움 -> 남은공간으로 인해 데이터가 추가될때 페이지 분할을 줄일수 있음
		OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF -- 순차적 키를 위한 특별한 최적화를 적용하지 않음
		) ON [PRIMARY]
	)
GO

--조회 빈도는 높지만 수정은 거의 없고, 정렬 기준도 아닐때 -> 비클러스터형인덱스
CREATE UNIQUE NONCLUSTERED INDEX IDX_Account_AccountUID
ON Account (AccountUID)
INCLUDE (AccountNumber)
GO

CREATE NONCLUSTERED INDEX IDX_Account_LastSignIndDate
ON Account (LastSignIndDate)
INCLUDE (AccountNumber)
GO

CREATE NONCLUSTERED INDEX IDX_Account_IsActive
ON Account (IsActive)
INCLUDE (AccountNumber)
GO

CREATE TABLE [dbo].[DeletedAccount]
	(
	[AccountNumber] [bigint] NOT NULL,
	[AccountUID] [varchar](55) NOT NULL,
	[CreateDate] [datetime] NOT NULL,
	[DeletedDate] [datetime] NOT NULL,
	CONSTRAINT [PK_DeletedAccount_AccountNumber] PRIMARY KEY CLUSTERED ([AccountNumber] ASC)
	WITH (
		PAD_INDEX = ON,
		STATISTICS_NORECOMPUTE = OFF,
		IGNORE_DUP_KEY = OFF,
		ALLOW_ROW_LOCKS = ON,
		ALLOW_PAGE_LOCKS = ON,
		FILLFACTOR = 90,
		OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF 
		) ON [PRIMARY]
	)
GO

CREATE TABLE [dbo].[UserData]
	(
	[AccountNumber] [bigint] NOT NULL,
	[AccountUID] [varchar](55) NOT NULL,
	[GameMoney] [bigint] NOT NULL,
	[GameMoneyRank] [int],
	[InventoryCapacity] [int] NOT NULL,
	[IsDeletedAccount] [tinyint] NOT NULL,
	CONSTRAINT [PK_UserData_AccountNumber] PRIMARY KEY CLUSTERED ([AccountNumber] ASC)
	WITH (
		PAD_INDEX = ON,
		STATISTICS_NORECOMPUTE = OFF,
		IGNORE_DUP_KEY = OFF,
		ALLOW_ROW_LOCKS = ON,
		ALLOW_PAGE_LOCKS = ON,
		FILLFACTOR = 90,
		OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF
		) ON [PRIMARY]
	)
GO

CREATE NONCLUSTERED INDEX IDX_UserData_IsDeletedAccount
ON UserData (IsDeletedAccount)
INCLUDE (AccountNumber);
GO


CREATE NONCLUSTERED INDEX IDX_UserData_GameMoneyRank
ON UserData (GameMoney DESC)
INCLUDE (AccountNumber)
WHERE IsDeletedAccount = 0;
GO

CREATE TABLE [dbo].[HistorySignInOut](
	[Seq] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountNumber] [bigint] NOT NULL,
	[CreateDate] [datetime] DEFAULT GETDATE(),
	[CreateDateYMD] AS CONVERT(VARCHAR(8), CreateDate, 112) PERSISTED,
	[SignInOut] [tinyint] NOT NULL, /* 0 = sign in, 1 = sign out */
	CONSTRAINT [PK_HistorySignInOut_Seq] PRIMARY KEY CLUSTERED ([Seq] ASC) 
	WITH (
    PAD_INDEX = ON,
    STATISTICS_NORECOMPUTE = OFF,
    IGNORE_DUP_KEY = OFF,
    ALLOW_ROW_LOCKS = ON,
    ALLOW_PAGE_LOCKS = ON,
    FILLFACTOR = 90,
    OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF 
) ON [PRIMARY]
)
GO

--계정별 in/out 날짜이력조회
CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_AccountNumber
ON HistorySignInOut (AccountNumber)
INCLUDE (SignInOut,CreateDate);
GO

--날짜별 in/out 계정조회
CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_CreateDateYMD
ON HistorySignInOut (CreateDateYMD)
INCLUDE (AccountNumber,SignInOut);
GO

CREATE TABLE [dbo].[HistoryUserMoney] (
    [Seq] [bigint] IDENTITY(1,1) NOT NULL, -- IDENTITY(시작값,증가값)
    [AccountNumber] BIGINT NOT NULL,
    [ChangeAmount] BIGINT NOT NULL,       -- 증감 금액 (+/-)
    [BeforeAmount] BIGINT NOT NULL,       -- 변경 전 금액
    [AfterAmount] BIGINT NOT NULL,        -- 변경 후 금액
    [Reason] NVARCHAR(200) NULL,          -- 변경 사유 (예: 구매, 보상 등)
    [CreateDate] DATETIME DEFAULT GETDATE(),
    [CreateDateYMD] AS CONVERT(VARCHAR(8), CreateDate, 112) PERSISTED
);
GO

--계정별 조회 인덱스
CREATE NONCLUSTERED INDEX IDX_HistoryUserMoney_AccountNumber
ON HistoryUserMoney (AccountNumber)
INCLUDE (CreateDate, ChangeAmount, BeforeAmount, AfterAmount, Reason);
GO

--날짜별 조회 인덱스
CREATE NONCLUSTERED INDEX IDX_HistoryUserMoney_CreateDate
ON HistoryUserMoney (CreateDate)
INCLUDE (AccountNumber, ChangeAmount, BeforeAmount, AfterAmount, Reason);
GO


--프로시저 정의

CREATE PROCEDURE CheckAndAddAccount
    @AccountUID VARCHAR(55)
AS
BEGIN
    SET NOCOUNT ON; -- 몇개의 행이 영향을 받았는지 출력하는 것을 OFF -> 불필요한 통신 최적화.

    DECLARE @Exists INT;
    DECLARE @AccountNumber BIGINT;

    SELECT @Exists = COUNT(*) FROM Account WHERE AccountUID = @AccountUID;

    IF @Exists > 0
	BEGIN
		SELECT @AccountNumber = AccountNumber 
		FROM Account 
		WHERE AccountUID = @AccountUID;

		SELECT 
			@AccountNumber AS AccountNumber,
			@AccountUID AS AccountUID,
			1 AS AccountExists;
		RETURN;
	END

    BEGIN TRY
        BEGIN TRANSACTION;

		-- Account에 INSERT하면서 생성된 AccountNumber를 바로 변수에 넣기
		DECLARE @AccountNumberTable TABLE (AccountNumber BIGINT);

		INSERT INTO Account (AccountUID, CreateDate, IsActive)
		OUTPUT INSERTED.AccountNumber INTO @AccountNumberTable
		VALUES (@AccountUID, GETDATE(), 0);

		-- 테이블 변수에서 실제 값 추출
		SELECT @AccountNumber = AccountNumber FROM @AccountNumberTable;

        -- 받은 AccountNumber로 UserData INSERT
        INSERT INTO UserData (AccountNumber, AccountUID, GameMoney, InventoryCapacity, IsDeletedAccount)
        VALUES (@AccountNumber, @AccountUID, 0, 100, 0);

        COMMIT;

        SELECT 
			@AccountNumber AS AccountNumber,
            @AccountUID AS AccountUID,
            0 AS AccountAdded;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;
        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

CREATE PROCEDURE DeleteAccount
    @AccountNumber BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    BEGIN TRY
        BEGIN TRANSACTION;

		IF NOT EXISTS (SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber)
			BEGIN
			ROLLBACK;
			RAISERROR(N'해당 AccountNumber가 존재하지 않습니다.', 16, 1);
			RETURN;
		END

        -- 1. 삭제될 Account 데이터를 DeletedAccount 테이블로 이동
        INSERT INTO DeletedAccount (AccountNumber, AccountUID, CreateDate, DeletedDate)
        SELECT AccountNumber, AccountUID, CreateDate, GETDATE()
        FROM Account
        WHERE AccountNumber = @AccountNumber;

        -- 2. Account에서 해당 레코드 삭제
        DELETE FROM Account
        WHERE AccountNumber = @AccountNumber;

        -- 3. UserData에서 삭제 표시
        UPDATE UserData
        SET IsDeletedAccount = 1
        WHERE AccountNumber = @AccountNumber;

        COMMIT;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END;
GO

CREATE PROCEDURE UserLogIn
    @AccountNumber BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Now DATETIME = GETDATE();

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 존재하지 않는 계정
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            COMMIT;  -- 트랜잭션은 열었으니 닫아주기
			SELECT
				@AccountNumber AS AccountNumber,
				3 AS ResultCode;  -- 계정 없음
            RETURN;
        END

        -- 이미 접속 중인 경우
        IF EXISTS (
            SELECT 1 FROM Account
            WHERE AccountNumber = @AccountNumber AND IsActive = 1
        )
        BEGIN
            COMMIT;
            SELECT
				@AccountNumber AS AccountNumber,
				2 AS ResultCode;  -- 이미 로그인 상태
            RETURN;
        END

        -- 접속 처리 (IsActive = 1 + 접속일시 갱신 + 히스토리 기록)
        UPDATE Account
        SET
            IsActive = 1,
            LastSignIndDate = @Now
        WHERE AccountNumber = @AccountNumber;

        INSERT INTO HistorySignInOut (AccountNumber, CreateDate, SignInOut)
        VALUES (@AccountNumber, @Now, 0);  -- 0 = sign in

        COMMIT;
        SELECT
			@AccountNumber AS AccountNumber,
			1 AS ResultCode;  -- 로그인 성공
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO


CREATE PROCEDURE UserLogOut
    @AccountNumber BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Now DATETIME = GETDATE();

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 존재하지 않는 계정
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            COMMIT;  -- 트랜잭션은 열었으니 닫아주기
            SELECT 3 AS ResultCode;  -- 계정 없음
            RETURN;
        END

        -- 접속중이지 않은 경우
        IF EXISTS (
            SELECT 1 FROM Account
            WHERE AccountNumber = @AccountNumber AND IsActive = 0
        )
        BEGIN
            COMMIT;
            SELECT 2 AS ResultCode;  -- 이미 로그아웃 상태
            RETURN;
        END

        -- 로그아웃 처리 (IsActive = 0  + 히스토리 기록)
        UPDATE Account
        SET
            IsActive = 0
        WHERE AccountNumber = @AccountNumber;

        INSERT INTO HistorySignInOut (AccountNumber, CreateDate, SignInOut)
        VALUES (@AccountNumber, @Now, 1);  -- 1 = sign out

        COMMIT;
        SELECT 1 AS ResultCode;  -- 로그아웃 성공
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

CREATE PROCEDURE GetAccountData
    @AccountNumber BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @AccountUID VARCHAR(55);
    DECLARE @GameMoney BIGINT;
    DECLARE @GameMoneyRank INT;
    DECLARE @InventoryCapacity INT;

    IF EXISTS (SELECT 1 FROM UserData WHERE AccountNumber = @AccountNumber)
    BEGIN
        SELECT 
            @AccountUID = AccountUID,
            @GameMoney = GameMoney,
            @GameMoneyRank = GameMoneyRank,
            @InventoryCapacity = InventoryCapacity
        FROM UserData
        WHERE AccountNumber = @AccountNumber;

        SELECT
            @AccountNumber AS AccountNumber,
            @AccountUID AS AccountUID,
            @GameMoney AS GameMoney,
            @GameMoneyRank AS GameMoneyRank,
            @InventoryCapacity AS InventoryCapacity,
            1 AS AccountExists;
    END
    ELSE
    BEGIN
        SELECT 
            NULL AS AccountNumber,
            NULL AS AccountUID,
            NULL AS GameMoney,
            NULL AS GameMoneyRank,
            NULL AS InventoryCapacity,
            0 AS AccountExists;
    END
END
GO

CREATE PROCEDURE UpdateUserMoney
    @AccountNumber BIGINT,
    @ChangedAmount BIGINT,
    @Sign TINYINT, -- 1 = 추가, 2 = 차감
    @Reason NVARCHAR(200) = N'수동 업데이트'
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Before BIGINT, @Result BIGINT;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. 계정 존재 확인
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            ROLLBACK;
            SELECT 0 AS ResultCode;  -- 계정 없음
            RETURN;
        END

        -- 2. 현재 금액 조회
        SELECT @Before = GameMoney
        FROM UserData WITH (UPDLOCK, ROWLOCK)
        WHERE AccountNumber = @AccountNumber;

        IF @Sign = 1
            SET @Result = @Before + @ChangedAmount;
        ELSE IF @Sign = 2
        BEGIN
            SET @Result = @Before - @ChangedAmount;
            IF @Result < 0
                SET @Result = 0;
        END
        ELSE
        BEGIN
            ROLLBACK;
            RAISERROR(N'@Sign 값은 1(증가), 2(차감) 중 하나여야 합니다.', 16, 1);
            RETURN;
        END

        -- 3. 금액 업데이트
        UPDATE UserData
        SET 
            GameMoney = @Result
        WHERE AccountNumber = @AccountNumber;

        -- 4. 히스토리 기록
        INSERT INTO HistoryUserMoney (AccountNumber, ChangeAmount, BeforeAmount, AfterAmount, Reason)
        VALUES (@AccountNumber, @ChangedAmount, @Before, @Result, @Reason);

        COMMIT;
        SELECT 1 AS ResultCode;  -- 성공
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END;
GO
