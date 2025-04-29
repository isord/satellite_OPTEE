# 기밀연산 인공위성 시스템 적용
## PQC 기반 보안 키 교환 및 OP-TEE 저장
- 인공위성 시스템에 OP-TEE를 기반으로 PQC 알고리즘을 접목시킨 기밀연산 프로토콜을 적용시켜 데이터를 보다 안전하게 관리한다.
- 인공위성 시스템에 OP-TEE 환경 구축
- OP-TEE 환경 내 PQC 알고리즘을 포팅
- 멤버 소개
  + 202002558 조민성
  + 201902711 신희성
  + 201802076 김주호

- **OP-TEE란?**
  
    ARM TrustZone 기술을 기반으로 동작하는 오픈 소스 보안 실행 환경(TEE)이다.
    신뢰할 수 있는 애플리케이션을 일반 OS와 분리된 안전한 영역에서 실행시켜 보안을 강화한다.

- **PQC 알고리즘이란?**

   PQC(Post-Quantum Cryptography) 알고리즘은 양자컴퓨터 시대에도 안전한 암호 기술이며,
   기존 RSA나 ECC가 양자컴퓨터에 취약한 점을 보완하기 위해 개발되었다.

## 7~8주차 활동
- 시퀀스다이어그램 작성하기
- 발표 url: [7~8주차 발표]()
- 제출물 : [시퀀스다이어그램](https://github.com/isord/satellite_OPTEE/blob/week7/docs/11%EC%A1%B0-7%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EC%8B%9C%ED%80%80%EC%8A%A4%EB%8B%A4%EC%9D%B4%EC%96%B4%EA%B7%B8%EB%9E%A8.pdf), [발표자료](https://github.com/isord/satellite_OPTEE/blob/00d7194ea44ed6e631b428c12001bde0202435e4/docs/ppt/11%E1%84%8C%E1%85%A9-7%E1%84%8C%E1%85%AE%E1%84%8E%E1%85%A1-%E1%84%80%E1%85%B5%E1%84%86%E1%85%B5%E1%86%AF%E1%84%8B%E1%85%A7%E1%86%AB%E1%84%89%E1%85%A1%E1%86%AB%20%E1%84%8B%E1%85%B5%E1%86%AB%E1%84%80%E1%85%A9%E1%86%BC%E1%84%8B%E1%85%B1%E1%84%89%E1%85%A5%E1%86%BC%20%E1%84%89%E1%85%B5%E1%84%89%E1%85%B3%E1%84%90%E1%85%A6%E1%86%B7%20%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%8B%E1%85%AD%E1%86%BC-%E1%84%87%E1%85%A1%E1%86%AF%E1%84%91%E1%85%AD%E1%84%8C%E1%85%A1%E1%84%85%E1%85%AD.pdf)
- PR 링크 : [7~8주차 PR]()

## 5~6주차 활동
- 유스케이스 작성하기
- 발표 url : [5~6주차 발표](https://youtu.be/f0kYec6o4E8?si=UH05Mx2l2RuBJluz)
- 제출물 : [유스케이스](https://github.com/isord/satellite_OPTEE/blob/week5/docs/11%EC%A1%B0-5%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EC%9C%A0%EC%8A%A4%EC%BC%80%EC%9D%B4%EC%8A%A4%EB%AA%85%EC%84%B8%EC%84%9C.pdf),[발표자료](https://github.com/isord/satellite_OPTEE/blob/main/docs/ppt/11%EC%A1%B0-6%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B0%9C%ED%91%9C%EC%9E%90%EB%A3%8C.pdf)
- PR 링크 : [5~6주차 PR](https://github.com/isord/satellite_OPTEE/pull/9)
  
## 4주차 활동
- 문제정의서 만들기
- 발표 url : [4주차 발표](https://youtu.be/dpTzdcrGmOc?si=dvOeL8yqa8pfDcGK)
- 제출물 : [문제정의서](https://github.com/isord/satellite_OPTEE/blob/week4/docs/11%EC%A1%B0-4%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%AC%B8%EC%A0%9C%EC%A0%95%EC%9D%98%EC%84%9C.pdf),[발표자료](https://github.com/isord/satellite_OPTEE/blob/week4/docs/ppt/11%EC%A1%B0-4%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B0%9C%ED%91%9C%EC%9E%90%EB%A3%8C.pdf)
- PR 링크 : [4주차 PR](https://github.com/isord/satellite_OPTEE/pull/8)
  
## 3주차 활동
- 브레인스토밍 결과 만들기
- 발표 url : [3주차 발표](https://www.youtube.com/watch?v=2vLsEb2MQSM&t=2s)
- 제출물 : [브레인스토밍 결과](https://github.com/isord/satellite_OPTEE/blob/week3/docs/11%EC%A1%B0-3%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B8%8C%EB%A0%88%EC%9D%B8%EC%8A%A4%ED%86%A0%EB%B0%8D%20%EA%B2%B0%EA%B3%BC.pdf)
,[발표 자료](https://github.com/isord/satellite_OPTEE/blob/week3/docs/ppt/11%EC%A1%B0-3%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B0%9C%ED%91%9C%EC%9E%90%EB%A3%8C.pdf)
- PR 링크 :[3주차 PR](https://github.com/isord/satellite_OPTEE/pull/7)

## 2주차 활동
- 문제점 목록 만들기
- 발표 url : [2주차 발표](https://www.youtube.com/watch?v=4JhA1YWM01Y)
- 제출물 : [문제점 목록](https://github.com/isord/satellite_OPTEE/blob/week2/docs/11%EC%A1%B0-2%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%AC%B8%EC%A0%9C%EC%A0%90%20%EB%AA%A9%EB%A1%9D.pdf) ,[발표 자료](https://github.com/isord/satellite_OPTEE/blob/week2/docs/ppt/11%EC%A1%B0-2%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B0%9C%ED%91%9C%EC%9E%90%EB%A3%8C.pdf)
-  PR 링크 : [2주차 PR](https://github.com/isord/satellite_OPTEE/pull/6)

## 1주차 활동
- 디자인 개요서 만들기
- 발표 url : [1주차 발표](https://youtu.be/3Tp8I-uHU-8)
- 제출물 : [디자인개요서](https://github.com/isord/satellite_OPTEE/blob/week2/docs/11%EC%A1%B0-1%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%94%94%EC%9E%90%EC%9D%B8%EA%B0%9C%EC%9A%94%EC%84%9C.pdf) ,[발표자료](https://github.com/isord/satellite_OPTEE/blob/week2/docs/ppt/11%EC%A1%B0-1%EC%A3%BC%EC%B0%A8-%EA%B8%B0%EB%B0%80%EC%97%B0%EC%82%B0%20%EC%9D%B8%EA%B3%B5%EC%9C%84%EC%84%B1%20%EC%8B%9C%EC%8A%A4%ED%85%9C%20%EC%A0%81%EC%9A%A9-%EB%B0%9C%ED%91%9C%EC%9E%90%EB%A3%8C.pdf)
- PR 링크 : [1주차 PR](https://github.com/isord/satellite_OPTEE/pull/3#issue-2921678320)
