CREATE DATABASE if NOT EXISTS mydb;
use mydb;

DROP TABLE if EXISTS user cascade;
DROP TABLE if EXISTS user_img cascade;
-- 用户表
CREATE TABLE if NOT EXISTS user (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,  -- 用户id，主键，自动递增
    username VARCHAR(32) UNIQUE,                 -- 用户名，唯一
    password VARCHAR(64)                         -- 密码
);

-- 图片表
CREATE TABLE if NOT EXISTS user_img (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,  -- 图片id，主键，自动递增
    username VARCHAR(32) NOT NULL,               -- 用户名，外键
    imagename VARCHAR(128) NOT NULL,             -- 图片名称
    imagepath VARCHAR(256),                      -- 图片路径
    FOREIGN KEY (username) REFERENCES user(username)
);
