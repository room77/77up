// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang), pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_BASE_SYSINFO_H_
#define _PUBLIC_BASE_SYSINFO_H_

class SysInfo  {
 public:
  virtual ~SysInfo() {};
  static SysInfo& Instance() {
    static SysInfo the_one;
    return the_one;
  }

  bool InProduction() const { return in_production_; }
  bool InTest() const { return in_test_; }
  bool InStaging() const { return in_staging_; }

 protected:
  SysInfo() { Init(); }

 private:
  void Init();
  bool in_production_;
  bool in_test_;
  bool in_staging_;
};

#endif  // _PUBLIC_BASE_SYSINFO_H_
