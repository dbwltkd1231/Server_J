*OpenSSL이 설치필요합니다.*  
OpenSSL설치방법  
cmd창에서  

<br>

git clone https://github.com/Microsoft/vcpkg.git  
cd vcpkg  
vcpkg install openssl  
vcpkg integrate install  

<br>

이후 비주얼스튜디오 켜져있었으면 껏다키면 됩니다.  


---
<br>
<br>
<br>

인증서버가 로비서버정보를 확인하고 클라이언트에 포트전달 및 연결해제되는 사진
<br>
![로비서버가 로비서버정보전달](https://github.com/user-attachments/assets/997a51db-b930-409b-9afa-639fea05bf56)

<br>

*유저접속기준이 아닌 로그인성공기준으로 로비서버정보를 저장하고있어, 접속인원100이 바로 99로 줄지않는 문제가 있습니다.
<br>
<br>
<br>
로비서버시작
<br>
![로비서버 시작](https://github.com/user-attachments/assets/2b028db0-90fb-4c39-9750-8a963d78ad78)
<br>
<br>
<br>

클라이언트가 로비서버연결을 위해 인증서버연결이 해제되는 사진
<br>
![클라이언트가 로비서버연결을위해 인증서버연결해제하는부분](https://github.com/user-attachments/assets/68a18eb3-77df-4ad6-b9e8-a9c44fc3f9ea)
<br>
<br>
<br>

로비서버가 유저로그인을 받는사진
<br>
![로비서버가 유저로그인받는부분](https://github.com/user-attachments/assets/db35dd89-c381-4ba4-881b-4380becd1d84)
<br>
<br>
<br>

클라이언트가 로그인요청성공후 로비서버로부터 유저정보를 받는 사진
<br>
![클라이언트가 로그인요청성공후 유저정보받는부분](https://github.com/user-attachments/assets/d1055d70-cd58-4a1c-aa0e-c31134793e6a)
<br>
<br>
<br>

60초마다 유저가 아이템을 지급받고 아이템분해 요청
<br>
![60초마다 아이템을받고 아이템분해](https://github.com/user-attachments/assets/7f5afc71-f012-47c8-981c-a7e372e16037)
<br>
<br>
<br>

로비서버가 유저로그아웃을 처리하는 사진
<br>
![로그아웃처리](https://github.com/user-attachments/assets/ab2238d7-1361-4d05-8534-db987a0de021)
<br>

<br>
<br>
<br>
<br>

프로젝트 관리   
<br>

클라이언트와 서버프로젝트를 개별 솔루션으로 각각 관리하던 방식 -> 클라이언트와 서버 프로젝트를 단일솔루션에서 관리하는 방식으로 수정하였습니다.
로비서버는 여러개를 켤수있도록 main함수에서 인자를 받도록 설정하였습니다.
쿼리문은 DB폴더에 저장하였습니다.

---


네트워크  
<br>

CustomOverlapped 객체를 Client에 멤버변수로 고정시켜놓고 char배열을 풀링해서 넣어주던 방식 -> CustomOverlapped객체에 char배열을 고정시켜놓고 CustomOverlapped를 풀링하는 방식으로 변경하였습니다.
네트워크연결에 필요한 Port,IP,BufferSize 등 정보중 컴파일전에 고정되는 값들은 Json파일로 저장후, ConstValue라는 스크립트에서 싱글톤으로 구현하여 전역접근이 가능하도록 구성해보았습니다. 'client_config.json'과 같은 파일입니다.


---


구현기능  
<br>

<br>
-- DB최신화
<br>
모든 DB정보의 갱신은 캐싱없이 즉각 이루어지도록 하였습니다.
따라서 항상 유저의 마지막상태가 저장됩니다.
<br>

<br>
-- 인증토큰  
<br>
로비서버와 인증서버가 모두 알고있는 SecretKey를 기준으로 인증토큰을 발급하고, 검사합니다.
OpenSSL를 활용하였습니다.
<br>

<br>
-- 동접 수에 따라 클라이언트가 접속할 로비 서버 선별   
<br>
로그인처리시 현재로그인처리된 유저의 수를 기준으로 로비서버가 Redis에 저장.
인증서버는 Redis에서 로비서버이름을 Key로 조회하여 그중 여유가 가장많은 포트번호를 클라이언트에 전송합니다.
<br>

<br>
-- 미인증 유저 연결해제  
<br>
Lobby서버에 연결된 소켓중 로그인이 안된 소켓은 60초이후 연결해제하도록 하였습니다.
<br>

<br>
-- 유저활동기록 저장  
<br>
로그인완료한 유저가 연결해제시 로그아웃처리 및 유저접속기록을 DB에 저장하였습니다.
아이템 분해시 변경되는 게임머니를 DB에 저장하였습니다.
<br>



---

<br>
시퀀스  
<br>

클라이언트가 계정고유번호를 인증서버에 전달.<br>
-> 인증서버는 DB에서 계정고유번호가 조회되지않으면 계정및 유저데이터기본값으로 저장 후 인증토큰 및 로비서버포트번호 발급.<br>
-> 인증토큰을 받은 클라이언트는 발급받은 로비서버포트번호를 통해 로비서버에 계정고유번호와 토큰을 전달. <br>
-> 로비서버는 토큰유효성확인후 성공시 최종 로그인처리. 이후 유저정보 및 인벤토리정보를 클라이언트에 전달.<br>
-> 로비서버는 최종로그인처리된 유저를 대상으로 60초마다 1~3개의 아이템을 지급한다.<br>
-> 유저는 아이템을 지급받았을때 랜덤한 확률로 로비서버에 아이템분해요청을 한다.<br>
-> 로비서버는 분해요청 처리후 바뀐 아이템정보, 유저의 게임머니정보를 유저에게 전송.
<br>
<br>
<br>
<br>
테스트했던 데이터베이스 예시 이미지
<br>

아이템
<br>
![Item](https://github.com/user-attachments/assets/bb454d79-350a-4d9a-bcf7-c7ef67ad4e2f)
<br>
<br>

인벤토리
<br>
![Inventory](https://github.com/user-attachments/assets/b768c22f-a005-4888-9d93-6f22c9a462b3)
<br>
<br>

유저정보
<br>
![AccountData](https://github.com/user-attachments/assets/78bda250-c6ca-43ef-b87c-138168ee88ba)
<br>
<br>

아이템 분해 히스토리
<br>
![ItemBreakHistory](https://github.com/user-attachments/assets/30742ce6-0ad8-4204-895a-1bcd2259f300)
<br>
<br>
유저 접속기록
<br>
![SignInHistory](https://github.com/user-attachments/assets/f5d53c3a-914a-4809-8ed5-9dff13b1d749)
<br>
<br>

계정정보
<br>
![Account](https://github.com/user-attachments/assets/4deb237d-305b-4273-97af-06ff7f0d0fd4)














