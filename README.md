OpenSSL설치방법
cmd창에서

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
vcpkg install openssl
vcpkg integrate install

이후 비주얼스튜디오 켜져있었으면 껏다키면 끝.


프로젝트 관리 
클라이언트와 서버프로젝트를 개별 솔루션으로 각각 관리하던 방식 -> 클라이언트와 서버 프로젝트를 단일솔루션에서 관리.
프로젝트가 동시에 사용하는 코드들은 SolutionDIr 경로에서 참조하도록 구성.



네트워크

1. CustomOverlapped 객체를 Client에 멤버변수로 고정시켜놓고 char배열을 풀링해서 넣어주던 방식 -> CustomOverlapped객체에 char배열을 고정시켜놓고 CustomOverlapped를 풀링하는 방식  <변경>
2. Port,IP,BufferSize 등 컴파일전에 고정되는 값들은 Json파일로 저장후, ConstValue라는 스크립트에서 싱글톤으로 구현하여 전역접근이 가능하도록 구성 <추가>
