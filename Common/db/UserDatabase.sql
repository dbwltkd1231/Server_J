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

--���̺� ����

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
	[IsActive] [tinyint] NOT NULL, -- ���� ��������:0, ������ : 1 
	CONSTRAINT [PK_Account_AccountNumber] PRIMARY KEY CLUSTERED ([AccountNumber] ASC)
	WITH (
		PAD_INDEX = ON, --���� ���� �ε��� ���������� ���� ������ Ȯ���ϵ��� ����
		STATISTICS_NORECOMPUTE = OFF,-- �ڵ����� ��������� ������Ʈ �ϵ��� ����
		IGNORE_DUP_KEY = OFF,--�ߺ� Ű �Է½� �����߻�
		ALLOW_ROW_LOCKS = ON,-- ����� ��� ��� -> ���ü� ����
		ALLOW_PAGE_LOCKS = ON,-- ������ ���� ��� ��� -> ���� �����͸� ���ÿ� ������Ʈ �� �� ���� ��� ����
		FILLFACTOR = 90,-- �ε��� �������� 90%��ä�� -> ������������ ���� �����Ͱ� �߰��ɶ� ������ ������ ���ϼ� ����
		OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF -- ������ Ű�� ���� Ư���� ����ȭ�� �������� ����
		) ON [PRIMARY]
	)
GO

--��ȸ �󵵴� ������ ������ ���� ����, ���� ���ص� �ƴҶ� -> ��Ŭ���������ε���
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

--������ in/out ��¥�̷���ȸ
CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_AccountNumber
ON HistorySignInOut (AccountNumber)
INCLUDE (SignInOut,CreateDate);
GO

--��¥�� in/out ������ȸ
CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_CreateDateYMD
ON HistorySignInOut (CreateDateYMD)
INCLUDE (AccountNumber,SignInOut);
GO

CREATE TABLE [dbo].[HistoryUserMoney] (
    [Seq] [bigint] IDENTITY(1,1) NOT NULL, -- IDENTITY(���۰�,������)
    [AccountNumber] BIGINT NOT NULL,
    [ChangeAmount] BIGINT NOT NULL,       -- ���� �ݾ� (+/-)
    [BeforeAmount] BIGINT NOT NULL,       -- ���� �� �ݾ�
    [AfterAmount] BIGINT NOT NULL,        -- ���� �� �ݾ�
    [Reason] NVARCHAR(200) NULL,          -- ���� ���� (��: ����, ���� ��)
    [CreateDate] DATETIME DEFAULT GETDATE(),
    [CreateDateYMD] AS CONVERT(VARCHAR(8), CreateDate, 112) PERSISTED
);
GO

--������ ��ȸ �ε���
CREATE NONCLUSTERED INDEX IDX_HistoryUserMoney_AccountNumber
ON HistoryUserMoney (AccountNumber)
INCLUDE (CreateDate, ChangeAmount, BeforeAmount, AfterAmount, Reason);
GO

--��¥�� ��ȸ �ε���
CREATE NONCLUSTERED INDEX IDX_HistoryUserMoney_CreateDate
ON HistoryUserMoney (CreateDate)
INCLUDE (AccountNumber, ChangeAmount, BeforeAmount, AfterAmount, Reason);
GO


--���ν��� ����

CREATE PROCEDURE CheckAndAddAccount
    @AccountUID VARCHAR(55)
AS
BEGIN
    SET NOCOUNT ON; -- ��� ���� ������ �޾Ҵ��� ����ϴ� ���� OFF -> ���ʿ��� ��� ����ȭ.

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

		-- Account�� INSERT�ϸ鼭 ������ AccountNumber�� �ٷ� ������ �ֱ�
		DECLARE @AccountNumberTable TABLE (AccountNumber BIGINT);

		INSERT INTO Account (AccountUID, CreateDate, IsActive)
		OUTPUT INSERTED.AccountNumber INTO @AccountNumberTable
		VALUES (@AccountUID, GETDATE(), 0);

		-- ���̺� �������� ���� �� ����
		SELECT @AccountNumber = AccountNumber FROM @AccountNumberTable;

        -- ���� AccountNumber�� UserData INSERT
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
			RAISERROR(N'�ش� AccountNumber�� �������� �ʽ��ϴ�.', 16, 1);
			RETURN;
		END

        -- 1. ������ Account �����͸� DeletedAccount ���̺�� �̵�
        INSERT INTO DeletedAccount (AccountNumber, AccountUID, CreateDate, DeletedDate)
        SELECT AccountNumber, AccountUID, CreateDate, GETDATE()
        FROM Account
        WHERE AccountNumber = @AccountNumber;

        -- 2. Account���� �ش� ���ڵ� ����
        DELETE FROM Account
        WHERE AccountNumber = @AccountNumber;

        -- 3. UserData���� ���� ǥ��
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

        -- �������� �ʴ� ����
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            COMMIT;  -- Ʈ������� �������� �ݾ��ֱ�
			SELECT
				@AccountNumber AS AccountNumber,
				3 AS ResultCode;  -- ���� ����
            RETURN;
        END

        -- �̹� ���� ���� ���
        IF EXISTS (
            SELECT 1 FROM Account
            WHERE AccountNumber = @AccountNumber AND IsActive = 1
        )
        BEGIN
            COMMIT;
            SELECT
				@AccountNumber AS AccountNumber,
				2 AS ResultCode;  -- �̹� �α��� ����
            RETURN;
        END

        -- ���� ó�� (IsActive = 1 + �����Ͻ� ���� + �����丮 ���)
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
			1 AS ResultCode;  -- �α��� ����
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

        -- �������� �ʴ� ����
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            COMMIT;  -- Ʈ������� �������� �ݾ��ֱ�
            SELECT 3 AS ResultCode;  -- ���� ����
            RETURN;
        END

        -- ���������� ���� ���
        IF EXISTS (
            SELECT 1 FROM Account
            WHERE AccountNumber = @AccountNumber AND IsActive = 0
        )
        BEGIN
            COMMIT;
            SELECT 2 AS ResultCode;  -- �̹� �α׾ƿ� ����
            RETURN;
        END

        -- �α׾ƿ� ó�� (IsActive = 0  + �����丮 ���)
        UPDATE Account
        SET
            IsActive = 0
        WHERE AccountNumber = @AccountNumber;

        INSERT INTO HistorySignInOut (AccountNumber, CreateDate, SignInOut)
        VALUES (@AccountNumber, @Now, 1);  -- 1 = sign out

        COMMIT;
        SELECT 1 AS ResultCode;  -- �α׾ƿ� ����
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
    @Sign TINYINT, -- 1 = �߰�, 2 = ����
    @Reason NVARCHAR(200) = N'���� ������Ʈ'
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Before BIGINT, @Result BIGINT;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. ���� ���� Ȯ��
        IF NOT EXISTS (
            SELECT 1 FROM Account WHERE AccountNumber = @AccountNumber
        )
        BEGIN
            ROLLBACK;
            SELECT 0 AS ResultCode;  -- ���� ����
            RETURN;
        END

        -- 2. ���� �ݾ� ��ȸ
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
            RAISERROR(N'@Sign ���� 1(����), 2(����) �� �ϳ����� �մϴ�.', 16, 1);
            RETURN;
        END

        -- 3. �ݾ� ������Ʈ
        UPDATE UserData
        SET 
            GameMoney = @Result
        WHERE AccountNumber = @AccountNumber;

        -- 4. �����丮 ���
        INSERT INTO HistoryUserMoney (AccountNumber, ChangeAmount, BeforeAmount, AfterAmount, Reason)
        VALUES (@AccountNumber, @ChangedAmount, @Before, @Result, @Reason);

        COMMIT;
        SELECT 1 AS ResultCode;  -- ����
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END;
GO
