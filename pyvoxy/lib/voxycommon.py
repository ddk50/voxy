
import ConfigParser
import StringIO



class VoxyConfig:
    def __init__(self):
        self.cp = ConfigParser.RawConfigParser()
        self.cp.read("targets.cfg")


    def list_targets(self):
        result = []
        for sec in self.cp.sections():
            if sec.startswith("target:"):
                result.append(sec[7:])
        return result

    def list_targets_for_user(self, user_name):
        return self.cp.get("user:" + user_name, "targets").split(',')

    def has_target(self, target_name):
        return self.cp.has_section("target:" + target_name)

    def get_target_options(self, target_name):
        result = {'name': target_name}
        for opt in self.cp.options("target:" + target_name):
            result[opt] = self.cp.get("target:" + target_name, opt)
        return result

    def get_command(self, target_name, cmd):
        return self.cp.get("target:" + target_name, "cmd_" + cmd)

    def has_user(self, user_name):
        return self.cp.has_section("user:" + user_name)

    def check_password_for_user(self, user_name, password):
        if not self.has_user(user_name):
            return False
        passwd_in_cfg = self.cp.get("user:" + user_name, "password")
        match_target = "!nomatch!"
        encoded_password = ""
        if passwd_in_cfg.startswith("str:"):
            encoded_password = "str:" + password
            match_target = passwd_in_cfg
        elif passwd_in_cfg.startswith("md5:"):
            encoded_password = "md5:" + hashlib.md5().update(password).hexdigest()
            encoded_password = encoded_password.lower()
            match_target = passwd_in_cfg.lower()

        print "PWD: ",encoded_password, match_target
        return encoded_password == match_target

    def get_target_host(self, target_name):
        return self.cp.get("target:" + target_name, "host")

    def get_target_port(self, target_name):
        return self.cp.getint("target:" + target_name, "port")

    def get_target_password(self, target_name):
        if not self.cp.has_option("target:" + target_name, "password"):
            return ""
        return self.cp.get("target:" + target_name, "password")

def GetConfig():
    return VoxyConfig()

