
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
	[AccountUID] [varchar](55) NOT NULL,
	[CreateDate] [datetime] NOT NULL,
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

CREATE NONCLUSTERED INDEX IDX_HistorySignInOut_AccountNumber_CreateDateYMD --��Ŭ�������� : ������ ������ �������� �ʰ� ������ �ε��� �������� ����
ON HistorySignInOut (AccountNumber, CreateDateYMD); -- ���� �÷� �ε����� ���� ������&��¥�� �˻� �����ϵ��� 
GO

CREATE TABLE [dbo].[HistoryUserMoney] (
    [Seq] [bigint] IDENTITY(1,1) NOT NULL,
    [AccountUID] VARCHAR(55) NOT NULL,
    [ChangeAmount] BIGINT NOT NULL,       -- ���� �ݾ� (+/-)
    [BeforeAmount] BIGINT NOT NULL,       -- ���� �� �ݾ�
    [AfterAmount] BIGINT NOT NULL,        -- ���� �� �ݾ�
    [Reason] NVARCHAR(200) NULL,          -- ���� ���� (��: ����, ���� ��)
    [CreateDate] DATETIME DEFAULT GETDATE()
);
GO

CREATE NONCLUSTERED INDEX IDX_UserMoneyLog_AccountUID_ChangedAt
ON HistoryUserMoney (AccountUID, CreateDate DESC);
GO


--���ν��� ����

CREATE PROCEDURE CheckAndAddAccount
    @AccountUID VARCHAR(55)
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Exists INT;

    -- 1. ������ ���� ���� Ȯ��
    SELECT @Exists = COUNT(*) FROM Account WHERE AccountUID = @AccountUID;

    IF @Exists > 0
    BEGIN
        -- �̹� �����ϸ� 1 ��ȯ
        SELECT 1 AS AccountExists;
        RETURN;
    END

    -- 2. �����Ͱ� ������ �߰� (Ʈ����� Ȱ��)
    BEGIN TRY
        BEGIN TRANSACTION;

        -- Account ���̺� �߰� (AccountNumber �ڵ� ����)
        INSERT INTO Account (AccountUID, CreateDate)
        VALUES (@AccountUID, GETDATE());

        -- UserData ���̺� �߰� (GameMoney �⺻�� 0)
        INSERT INTO UserData (AccountUID, GameMoney, LastUpdate)
        VALUES (@AccountUID, 0, GETDATE());

        COMMIT;

        -- �߰� ���� �� 0 ��ȯ
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

    -- Account ���� �� ON DELETE CASCADE�� ����� UserData �ڵ� ����
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
    VALUES (@AccountNumber, GETDATE(), @SignInOut);/* 112�� SQL Server�� ��¥ ���� ��ȯ �ڵ� */
END
GO

CREATE PROCEDURE UpdateUserMoney
    @AccountUID VARCHAR(55),
    @GameMoney BIGINT,
    @Reason NVARCHAR(200) = N'���� ������Ʈ'
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @Before BIGINT, @ChangeAmount BIGINT;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. ���� �ݾ� ��ȸ (������Ʈ ��� + �� ���)
        SELECT @Before = GameMoney
        FROM UserData WITH (UPDLOCK, ROWLOCK)
        WHERE AccountUID = @AccountUID;

        -- �������� �ʴ� ���� ó��
        IF @Before IS NULL
        BEGIN
            RAISERROR('�������� �ʴ� �����Դϴ�.', 16, 1);
            ROLLBACK;
            RETURN;
        END

        SET @ChangeAmount = @GameMoney - @Before;

        -- 2. ���ӸӴ� ������Ʈ
        UPDATE UserData
        SET 
            GameMoney = @GameMoney,
            LastUpdate = GETDATE()
        WHERE AccountUID = @AccountUID;

        -- 3. �����丮 ���
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
