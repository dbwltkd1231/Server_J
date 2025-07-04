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

CREATE NONCLUSTERED INDEX IX_Inventory_Account_Item
ON [dbo].[Inventory] ([AccountNumber], [ItemSeed]);
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

    --  변경된 슬롯들을 추적할 테이블 변수
    DECLARE @ChangedSlots TABLE (
        Guid UNIQUEIDENTIFIER,
        ItemSeed BIGINT,
        ItemCount INT
    );

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. 아이템 적재 정보 로드
        SELECT 
            @IsPile = IsPile,
            @PileMax = CASE WHEN IsPile = 0 THEN 1 ELSE PileCountMax END
        FROM Item
        WHERE ItemSeed = @ItemSeed;

        -- 2. 적재 루프
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

                -- 수정된 슬롯을 기록
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

        -- 최종적으로 모든 변경된 슬롯 반환
        SELECT * FROM @ChangedSlots;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        -- 에러 시 NULL 행 반환
        SELECT NULL AS Guid, @ItemSeed AS ItemSeed, NULL AS ItemCount;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

CREATE PROCEDURE BreakInventoryItem
    @AccountNumber BIGINT,
    @Guid UNIQUEIDENTIFIER,
    @RemoveCount INT
AS
BEGIN
    SET NOCOUNT ON;

    BEGIN TRY
        BEGIN TRANSACTION;

        DECLARE @ItemSeed BIGINT;
        DECLARE @ItemCount INT;

        -- 대상 인벤토리 슬롯 조회
        SELECT @ItemSeed = ItemSeed, @ItemCount = ItemCount
        FROM [dbo].[Inventory] WITH (UPDLOCK, ROWLOCK)
        WHERE [Guid] = @Guid;

        -- 슬롯이 없거나 수량 부족일 경우 실패
        IF @ItemSeed IS NULL OR @ItemCount < @RemoveCount
        BEGIN
            ROLLBACK;
            SELECT 
                0 AS ResultCode,
                @Guid AS Guid,
                0 AS MoneyReward,
                0 AS RemoveCount;
            RETURN;
        END

        -- 계정 불일치일 경우 실패
        IF EXISTS (
            SELECT 1 FROM [dbo].[Inventory]
            WHERE [Guid] = @Guid AND [AccountNumber] <> @AccountNumber
        )
        BEGIN
            ROLLBACK;
            SELECT 
                0 AS ResultCode,
                @Guid AS Guid,
                0 AS MoneyReward,
                0 AS RemoveCount;
            RETURN;
        END

        -- 수량 차감 또는 행 삭제
        IF @ItemCount = @RemoveCount
        BEGIN
            DELETE FROM [dbo].[Inventory] WHERE [Guid] = @Guid;
        END
        ELSE
        BEGIN
            UPDATE [dbo].[Inventory]
            SET [ItemCount] = [ItemCount] - @RemoveCount
            WHERE [Guid] = @Guid;
        END

        -- 아이템 보상 계산
        DECLARE @BreakMoneyAmount INT;

        SELECT @BreakMoneyAmount = BreakMoneyAmount
        FROM [dbo].[Item]
        WHERE [ItemSeed] = @ItemSeed;

        DECLARE @MoneyReward BIGINT = @RemoveCount * @BreakMoneyAmount;

        EXEC [User].dbo.UpdateUserMoney
            @AccountNumber = @AccountNumber,
            @ChangedAmount = @MoneyReward,
            @Sign = 1,
            @Reason = N'아이템 해체 보상';

        COMMIT;

        SELECT 
            1 AS ResultCode,
            @Guid AS Guid,
            @MoneyReward AS MoneyReward,
            @RemoveCount AS RemoveCount;
    END TRY
    BEGIN CATCH
        IF @@TRANCOUNT > 0 ROLLBACK;

        DECLARE @ErrMsg NVARCHAR(4000), @ErrSeverity INT;
        SELECT @ErrMsg = ERROR_MESSAGE(), @ErrSeverity = ERROR_SEVERITY();
        RAISERROR(@ErrMsg, @ErrSeverity, 1);
    END CATCH
END
GO

--Sample Data

INSERT INTO [dbo].[Item] (ItemSeed, IsPile, PileCountMax, BreakMoneyAmount) VALUES
(10001, 1, 2, 500),
(10002, 1, 5, 2000),
(10003, 0, 1, 5000),
(10004, 1, 10, 50),
(10005, 0, 1, 0);

