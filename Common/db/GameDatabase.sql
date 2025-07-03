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

CREATE UNIQUE NONCLUSTERED INDEX IDX_Inventory_AccountNumber_ItemSeed
ON Inventory (AccountNumber,ItemSeed)
INCLUDE (Guid, ItemCount);
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
    DECLARE @Remaining INT = @AddCount;

    BEGIN TRY
        BEGIN TRANSACTION;

        -- 1. 아이템 존재 여부 확인
        IF NOT EXISTS (SELECT 1 FROM Item WHERE ItemSeed = @ItemSeed)
        BEGIN
            RAISERROR(N'해당 Item이 존재하지 않습니다.', 16, 1);
            ROLLBACK;
            RETURN;
        END

        -- 2. 최대 적재 수량 설정 (IsPile = 0이면 무조건 1로 제한)
        SELECT 
            @PileMax = CASE 
                          WHEN IsPile = 0 THEN 1
                          ELSE PileCountMax
                       END
        FROM Item 
        WHERE ItemSeed = @ItemSeed;

        -- 3. 인벤토리 적재 로직
        WHILE @Remaining > 0
        BEGIN
            DECLARE @TargetGuid UNIQUEIDENTIFIER;
            DECLARE @CurrentCount INT;

            -- 기존 슬롯 중 빈 공간 찾기
		SELECT TOP 1 
			@TargetGuid = Guid, 
			@CurrentCount = ItemCount
		FROM Inventory WITH (UPDLOCK, ROWLOCK)
		WHERE AccountNumber = @AccountNumber
			AND ItemSeed = @ItemSeed
			AND ItemCount < @PileMax
		ORDER BY ItemCount ASC;  -- 가장 적게 쌓인 슬롯부터 선택

            IF @TargetGuid IS NOT NULL
            BEGIN
                DECLARE @SpaceLeft INT = @PileMax - @CurrentCount;
                DECLARE @AddToSlot INT = CASE 
                                             WHEN @Remaining > @SpaceLeft THEN @SpaceLeft 
                                             ELSE @Remaining 
                                         END;

                UPDATE Inventory
                SET ItemCount = ItemCount + @AddToSlot
                WHERE Guid = @TargetGuid;

                SET @Remaining = @Remaining - @AddToSlot;
            END
            ELSE
            BEGIN
                DECLARE @InsertCount INT = CASE 
                                               WHEN @Remaining >= @PileMax THEN @PileMax 
                                               ELSE @Remaining 
                                           END;

                INSERT INTO Inventory (AccountNumber, ItemSeed, ItemCount)
                VALUES (@AccountNumber, @ItemSeed, @InsertCount);

                SET @Remaining = @Remaining - @InsertCount;
            END
        END

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

            -- 제거 가능한 슬롯 탐색
            SELECT TOP 1
                @TargetGuid = Guid,
                @CurrentCount = ItemCount
            FROM Inventory WITH (UPDLOCK, ROWLOCK)
            WHERE AccountNumber = @AccountNumber
              AND ItemSeed = @ItemSeed
            ORDER BY ItemCount ASC;

            -- 슬롯이 없는데 남은 수량이 있다면 실패로 롤백
            IF @TargetGuid IS NULL
            BEGIN
                ROLLBACK;
                SELECT 0 AS ResultCode;  -- 제거 실패
                RETURN;
            END

            -- 슬롯 수량보다 많이 빼야 하면 해당 슬롯 삭제
            IF @CurrentCount <= @Remaining
            BEGIN
                DELETE FROM Inventory WHERE Guid = @TargetGuid;
                SET @Remaining = @Remaining - @CurrentCount;
            END
            ELSE
            BEGIN
                -- 일부만 차감
                UPDATE Inventory
                SET ItemCount = ItemCount - @Remaining
                WHERE Guid = @TargetGuid;

                SET @Remaining = 0;
            END
        END

        COMMIT;
        SELECT 1 AS ResultCode;  -- 성공
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

        -- 1. 인벤토리에서 아이템 제거 시도
        EXEC [Game].dbo.DeleteInventoryItem
            @AccountNumber = @AccountNumber,
            @ItemSeed = @ItemSeed,
            @RemoveCount = @RemoveCount;

        -- 제거 실패 시 (슬롯 부족)
        IF @@ROWCOUNT = 0
        BEGIN
            ROLLBACK;
            SELECT 0 AS ResultCode; -- 해체 실패
            RETURN;
        END

        -- 2. 해체 수만큼 게임머니 100원씩 지급
        DECLARE @MoneyReward BIGINT = @RemoveCount * 100;

        EXEC [User].dbo.UpdateUserMoney
            @AccountNumber = @AccountNumber,
            @ChangedAmount = @MoneyReward,
            @Sign = 1,  -- 증가
            @Reason = N'아이템 해체 보상';

        COMMIT;
        SELECT 1 AS ResultCode; -- 성공
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

