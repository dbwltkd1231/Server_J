USE [master]
GO

CREATE DATABASE [Game]
ON PRIMARY
( NAME = N'Game', FILENAME = N'C:\Program Files\Microsoft SQL Server\MSSQL16.SQLEXPRESS\MSSQL\DATA\Game.mdf',
SIZE = 102400KB, MAXSIZE = UNLIMITED, FILEGROWTH = 10240KB )
LOG ON
( NAME = N'Game_Log', FILENAME = N'C:\Program Files\Microsoft SQL Server\MSSQL16.SQLEXPRESS\MSSQL\DATA\Game_Log.ldf',
SIZE = 102400KB, MAXSIZE = 2048GB, FILEGROWTH = 10240KB )
GO

ALTER DATABASE [Game] SET COMPATIBILITY_LEVEL = 100
GO


USE [Game]
GO

CREATE TABLE [dbo].[Item]
	(
	[ItemSeed] [bigint] NOT NULL,
	[IsPile] [tinyint] NOT NULL,
	[PileCountMax] [int] NOT NULL,
	[BreakMoneyAmount] [int] NOT NULL,
	CONSTRAINT [PK_Item_ItemSeed] PRIMARY KEY CLUSTERED ([ItemSeed] ASC)
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


CREATE TABLE [dbo].[Inventory]
	(
	[Guid] [uniqueidentifier] NOT NULL DEFAULT NEWSEQUENTIALID(),
	[AccountNumber] [bigint] NOT NULL,
	[ItemSeed] [bigint] NOT NULL,
	[ItemCount] [int] NOT NULL,
	CONSTRAINT [PK_Inventory_Guid] PRIMARY KEY CLUSTERED ([Guid] ASC)
	WITH (
		PAD_INDEX = ON, 
		STATISTICS_NORECOMPUTE = OFF,
		IGNORE_DUP_KEY = OFF,
		ALLOW_ROW_LOCKS = ON,
		ALLOW_PAGE_LOCKS = ON,
		FILLFACTOR = 90,
		OPTIMIZE_FOR_SEQUENTIAL_KEY = ON
		) ON [PRIMARY]
	)
GO


ALTER TABLE Inventory
ADD CONSTRAINT FK_Inventory_Item
FOREIGN KEY (ItemSeed)
REFERENCES Item(ItemSeed)
ON DELETE CASCADE;
GO

--procedure

CREATE PROCEDURE GetItemAllData
AS
BEGIN
    SET NOCOUNT ON;

    SELECT 
        ItemSeed,
        IsPile,
        PileCountMax,
        BreakMoneyAmount
    FROM Item
    ORDER BY ItemSeed ASC;
END
GO

CREATE PROCEDURE GetInventoryByAccount
    @AccountNumber BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    SELECT 
        Guid,
        ItemSeed,
        ItemCount
    FROM dbo.Inventory
    WHERE AccountNumber = @AccountNumber
    ORDER BY ItemSeed ASC;
END
GO

CREATE PROCEDURE AddInventoryItem
    @AccountNumber BIGINT,
    @ItemSeed BIGINT,
    @AddCount INT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @PileMax INT;
    DECLARE @IsPile BIT;
    DECLARE @Remaining INT = @AddCount;

    --  ����� ���Ե��� ������ ���̺� ����
    DECLARE @ChangedSlots TABLE (
        Guid UNIQUEIDENTIFIER,
        ItemSeed BIGINT,
        ItemCount INT
    );

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. ������ ���� ���� �ε�
        SELECT 
            @IsPile = IsPile,
            @PileMax = CASE WHEN IsPile = 0 THEN 1 ELSE PileCountMax END
        FROM Item
        WHERE ItemSeed = @ItemSeed;

        -- 2. ���� ����
        WHILE @Remaining > 0
        BEGIN
            DECLARE @SlotGuid UNIQUEIDENTIFIER;
            DECLARE @CurrentCount INT;

            IF @IsPile = 1
            BEGIN
                SELECT TOP 1
                    @SlotGuid = Guid,
                    @CurrentCount = ItemCount
                FROM Inventory WITH (UPDLOCK, ROWLOCK)
                WHERE AccountNumber = @AccountNumber
                  AND ItemSeed = @ItemSeed
                  AND ItemCount < @PileMax
                ORDER BY ItemCount ASC;
            END
            ELSE
            BEGIN
                SET @SlotGuid = NULL;
            END

            IF @SlotGuid IS NOT NULL
            BEGIN
                DECLARE @AddNow INT = IIF(@Remaining > (@PileMax - @CurrentCount), @PileMax - @CurrentCount, @Remaining);

                UPDATE Inventory
                SET ItemCount = ItemCount + @AddNow
                WHERE Guid = @SlotGuid;

                SET @Remaining -= @AddNow;

                -- ������ ������ ���
                INSERT INTO @ChangedSlots
                SELECT Guid, ItemSeed, ItemCount FROM Inventory WHERE Guid = @SlotGuid;
            END
            ELSE
            BEGIN
                DECLARE @NewCount INT = IIF(@Remaining >= @PileMax, @PileMax, @Remaining);
                SET @Remaining -= @NewCount;

                DECLARE @NewGuid UNIQUEIDENTIFIER = NEWID();

                INSERT INTO Inventory (Guid, AccountNumber, ItemSeed, ItemCount)
                VALUES (@NewGuid, @AccountNumber, @ItemSeed, @NewCount);

                INSERT INTO @ChangedSlots VALUES (@NewGuid, @ItemSeed, @NewCount);
            END
        END

        COMMIT;

        -- ���������� ��� ����� ���� ��ȯ
        SELECT * FROM @ChangedSlots;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        -- ���� �� NULL �� ��ȯ
        SELECT NULL AS Guid, @ItemSeed AS ItemSeed, NULL AS ItemCount;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

CREATE PROCEDURE DeleteInventoryItem
    @AccountNumber BIGINT,
    @ItemSeed BIGINT,
    @RemoveCount INT
AS
BEGIN
    SET NOCOUNT ON;

    BEGIN TRY
        BEGIN TRANSACTION;

        DECLARE @Remaining INT = @RemoveCount;

        WHILE @Remaining > 0
        BEGIN
            DECLARE @TargetGuid UNIQUEIDENTIFIER;
            DECLARE @CurrentCount INT;

            -- ���� ������ ���� Ž��
            SELECT TOP 1
                @TargetGuid = Guid,
                @CurrentCount = ItemCount
            FROM Inventory WITH (UPDLOCK, ROWLOCK)
            WHERE AccountNumber = @AccountNumber
              AND ItemSeed = @ItemSeed
            ORDER BY ItemCount ASC;

            -- ������ ���µ� ���� ������ �ִٸ� ���з� �ѹ�
            IF @TargetGuid IS NULL
            BEGIN
                ROLLBACK;
                SELECT 0 AS ResultCode;  -- ���� ����
                RETURN;
            END

            -- ���� �������� ���� ���� �ϸ� �ش� ���� ����
            IF @CurrentCount <= @Remaining
            BEGIN
                DELETE FROM Inventory WHERE Guid = @TargetGuid;
                SET @Remaining = @Remaining - @CurrentCount;
            END
            ELSE
            BEGIN
                -- �Ϻθ� ����
                UPDATE Inventory
                SET ItemCount = ItemCount - @Remaining
                WHERE Guid = @TargetGuid;

                SET @Remaining = 0;
            END
        END

        COMMIT;
        SELECT 1 AS ResultCode;  -- ����
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

CREATE PROCEDURE BreakInventoryItem
    @AccountNumber BIGINT,
    @ItemSeed BIGINT,
    @RemoveCount INT
AS
BEGIN
    SET NOCOUNT ON;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. �κ��丮���� ������ ���� �õ�
        EXEC [Game].dbo.DeleteInventoryItem
            @AccountNumber = @AccountNumber,
            @ItemSeed = @ItemSeed,
            @RemoveCount = @RemoveCount;

        -- ���� ���� �� (���� ����)
        IF @@ROWCOUNT = 0
        BEGIN
            ROLLBACK;
            SELECT 0 AS ResultCode; -- ��ü ����
            RETURN;
        END

        -- 2. ��ü ����ŭ ���ӸӴ� 100���� ����
        DECLARE @MoneyReward BIGINT = @RemoveCount * 100;

        EXEC [User].dbo.UpdateUserMoney
            @AccountNumber = @AccountNumber,
            @ChangedAmount = @MoneyReward,
            @Sign = 1,  -- ����
            @Reason = N'������ ��ü ����';

        COMMIT;
        SELECT 1 AS ResultCode; -- ����
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END;
GO

--Sample Data

INSERT INTO [dbo].[Item] (ItemSeed, IsPile, PileCountMax, BreakMoneyAmount) VALUES
(10001, 1, 2, 500),
(10002, 1, 5, 2000),
(10003, 0, 1, 5000),
(10004, 1, 10, 50),
(10005, 0, 1, 0);

