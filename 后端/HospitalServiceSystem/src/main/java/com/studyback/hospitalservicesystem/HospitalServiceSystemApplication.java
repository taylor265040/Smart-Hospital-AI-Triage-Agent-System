package com.studyback.hospitalservicesystem;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
@MapperScan("com.studyback.hospitalservicesystem.mapper")
public class HospitalServiceSystemApplication {

    public static void main(String[] args) {
        SpringApplication.run(HospitalServiceSystemApplication.class, args);
    }

}
