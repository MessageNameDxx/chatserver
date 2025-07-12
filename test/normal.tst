注册
{"msgid": 3, "name": "dxx", "password": "123456"}
{"msgid": 3, "name": "x", "password": "123456"}

登陆
{"msgid":1,"id":1,"password":"123456"}
{"msgid":1,"id":2,"password":"123456"}

聊天
{"msgid":5,"id":1,"from":"xx","to":2,"msg":"hello"}
{"msgid":5,"id":2,"from":"dxx","to":1,"msg":"i love you"}

添加好友
{"msgid":6,"id":1,"friendid":2}
{"msgid":6,"id":2,"friendid":1}


========================================================================================================================

CREATE TABLE User (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(100) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline'
    );

CREATE TABLE offlineMessage (
    userid INT PRIMARY KEY AUTO_INCREMENT,
    namemessage VARCHAR(500) NOT NULL,
    );

CREATE TABLE Friend (
    userid INT NOT NULL,
    friendid INT NOT NULL,
    PRIMARY KEY (userid, friendid)
);


SELECT a.id,a.name,a.state from user a inner join friend b on b.userid = a.id where a,userid = %d;

CREATE TABLE allGroup (
    id INT PRIMARY KEY AUTO_INCREMENT,
    groupname VARCHAR(50) NOT NULL UNIQUE,
    groupdesc VARCHAR(200) DEFAULT ''
    );

CREATE TABLE groupUser (
    groupid INT NOT NULL,
    userid INT NOT NULL,
    grouprole ENUM('creator', 'normal') DEFAULT 'normal',
    PRIMARY KEY (groupid, userid)
);
