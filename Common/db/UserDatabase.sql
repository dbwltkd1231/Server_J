
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
	[AccountUID] [varchar](55) NOT NULL,
	[CreateDate] [datetime] NOT NULL,
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
ON Account (AccountUID);
GO


CREATE TABLE [dbo].[UserData]
	(
	[AccountUID] [varchar](55) NOT NULL,
	[GameMoney] [bigint] NOT NULL,
	[LastUpdate] [datetime],
	CONSTRAINT [PK_UserData_AccountUID] PRIMARY KEY CLUSTERED ([AccountUID] ASC)
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

CREATE NONCLUSTERED INDEX IDX_UserData_GameMoney 
ON UserData (GameMoney); 
GO

ALTER TABLE UserData
ADD CONSTRAINT FK_UserData_Account
FOREIGN KEY (AccountUID)
REFERENCES Account(AccountUID)
ON DELETE CASCADE;
GO


CREATE TABLE [dbo].[HistorySignInOut](
	[Seq] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountNumber] [bigint] NOT NULL,
	[CreateDate] [datetime] NOT NULL,
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

CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_AccountNumber_CreateDateYMD --비클러스터형 : 데이터 정렬을 변경하지 않고 별도의 인덱스 페이지를 유지
ON HistorySignInOut (AccountNumber, CreateDateYMD); -- 다중 컬럼 인덱스를 통한 계정별&날짜별 검색 용이하도록 
GO

CREATE TABLE [dbo].[HistoryUserMoney] (
    [Seq] [bigint] IDENTITY(1,1) NOT NULL,
    [AccountUID] VARCHAR(55) NOT NULL,
    [ChangeAmount] BIGINT NOT NULL,       -- 증감 금액 (+/-)
    [BeforeAmount] BIGINT NOT NULL,       -- 변경 전 금액
    [AfterAmount] BIGINT NOT NULL,        -- 변경 후 금액
    [Reason] NVARCHAR(200) NULL,          -- 변경 사유 (예: 구매, 보상 등)
    [CreateDate] DATETIME DEFAULT GETDATE()
);
GO

CREATE NONCLUSTERED INDEX IDX_UserMoneyLog_AccountUID_ChangedAt
ON HistoryUserMoney (AccountUID, CreateDate DESC);
GO


--프로시저 정의

CREATE PROCEDURE CheckAndAddAccount
    @AccountUID VARCHAR(55)
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Exists INT;

    -- 1. 데이터 존재 여부 확인
    SELECT @Exists = COUNT(*) FROM Account WHERE AccountUID = @AccountUID;

    IF @Exists > 0
    BEGIN
        -- 이미 존재하면 1 반환
        SELECT 1 AS AccountExists;
        RETURN;
    END

    -- 2. 데이터가 없으면 추가 (트랜잭션 활용)
    BEGIN TRY
        BEGIN TRANSACTION;

        -- Account 테이블에 추가 (AccountNumber 자동 생성)
        INSERT INTO Account (AccountUID, CreateDate)
        VALUES (@AccountUID, GETDATE());

        -- UserData 테이블에 추가 (GameMoney 기본값 0)
        INSERT INTO UserData (AccountUID, GameMoney, LastUpdate)
        VALUES (@AccountUID, 0, GETDATE());

        COMMIT;

        -- 추가 성공 시 0 반환
        SELECT 0 AS AccountAdded;
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
    @AccountUID VARCHAR(55)
AS
BEGIN
    SET NOCOUNT ON;

    -- Account 삭제 → ON DELETE CASCADE로 연결된 UserData 자동 삭제
    DELETE FROM Account WHERE AccountUID = @AccountUID;
END
GO


CREATE PROCEDURE AddHistorySignInOut
    @AccountNumber INT,
	@SignInOut INT
AS
BEGIN
    SET NOCOUNT ON;

    INSERT INTO HistorySignInOut (AccountNumber, CreateDate, SignInOut)
    VALUES (@AccountNumber, GETDATE(), @SignInOut);/* 112는 SQL Server의 날짜 형식 변환 코드 */
END
GO

CREATE PROCEDURE UpdateUserMoney
    @AccountUID VARCHAR(55),
    @GameMoney BIGINT,
    @Reason NVARCHAR(200) = N'수동 업데이트'
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Before BIGINT, @ChangeAmount BIGINT;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. 현재 금액 조회 (업데이트 잠금 + 행 잠금)
        SELECT @Before = GameMoney
        FROM UserData WITH (UPDLOCK, ROWLOCK)
        WHERE AccountUID = @AccountUID;

        -- 존재하지 않는 계정 처리
        IF @Before IS NULL
        BEGIN
            RAISERROR('존재하지 않는 계정입니다.', 16, 1);
            ROLLBACK;
            RETURN;
        END

        SET @ChangeAmount = @GameMoney - @Before;

        -- 2. 게임머니 업데이트
        UPDATE UserData
        SET 
            GameMoney = @GameMoney,
            LastUpdate = GETDATE()
        WHERE AccountUID = @AccountUID;

        -- 3. 히스토리 기록
        INSERT INTO HistoryUserMoney (AccountUID, ChangeAmount, BeforeAmount, AfterAmount, Reason)
        VALUES (@AccountUID, @ChangeAmount, @Before, @GameMoney, @Reason);

        COMMIT;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO
