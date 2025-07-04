*OpenSSL이 설치필요합니다.*
OpenSSL설치방법
cmd창에서

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
vcpkg install openssl
vcpkg integrate install

이후 비주얼스튜디오 켜져있었으면 껏다키면 됩니다.


프로젝트 관리 
클라이언트와 서버프로젝트를 개별 솔루션으로 각각 관리하던 방식 -> 클라이언트와 서버 프로젝트를 단일솔루션에서 관리하는 방식으로 수정하였습니다.
로비서버는 여러개를 켤수있도록 main함수에서 인자를 받도록 설정하였습니다.
쿼리문은 DB폴더에 저장하였습니다.

네트워크
CustomOverlapped 객체를 Client에 멤버변수로 고정시켜놓고 char배열을 풀링해서 넣어주던 방식 -> CustomOverlapped객체에 char배열을 고정시켜놓고 CustomOverlapped를 풀링하는 방식으로 변경하였습니다.
네트워크연결에 필요한 Port,IP,BufferSize 등 정보중 컴파일전에 고정되는 값들은 Json파일로 저장후, ConstValue라는 스크립트에서 싱글톤으로 구현하여 전역접근이 가능하도록 구성해보았습니다. 'client_config.json'과 같은 파일입니다.


