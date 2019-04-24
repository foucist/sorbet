# -*- encoding: utf-8 -*-
# stub: rack-oauth2 1.9.3 ruby lib

Gem::Specification.new do |s|
  s.name = "rack-oauth2".freeze
  s.version = "1.9.3"

  s.required_rubygems_version = Gem::Requirement.new(">= 0".freeze) if s.respond_to? :required_rubygems_version=
  s.require_paths = ["lib".freeze]
  s.authors = ["nov matake".freeze]
  s.date = "2018-10-29"
  s.description = "OAuth 2.0 Server & Client Library. Both Bearer and MAC token type are supported.".freeze
  s.email = "nov@matake.jp".freeze
  s.extra_rdoc_files = ["LICENSE".freeze, "README.rdoc".freeze]
  s.files = ["LICENSE".freeze, "README.rdoc".freeze]
  s.homepage = "http://github.com/nov/rack-oauth2".freeze
  s.licenses = ["MIT".freeze]
  s.rdoc_options = ["--charset=UTF-8".freeze]
  s.rubygems_version = "2.7.7".freeze
  s.summary = "OAuth 2.0 Server & Client Library - Both Bearer and MAC token type are supported".freeze

  s.installed_by_version = "2.7.7" if s.respond_to? :installed_by_version

  if s.respond_to? :specification_version then
    s.specification_version = 4

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_runtime_dependency(%q<rack>.freeze, [">= 0"])
      s.add_runtime_dependency(%q<httpclient>.freeze, [">= 0"])
      s.add_runtime_dependency(%q<activesupport>.freeze, [">= 0"])
      s.add_runtime_dependency(%q<attr_required>.freeze, [">= 0"])
      s.add_runtime_dependency(%q<json-jwt>.freeze, [">= 1.9.0"])
      s.add_development_dependency(%q<rake>.freeze, [">= 0"])
      s.add_development_dependency(%q<simplecov>.freeze, [">= 0"])
      s.add_development_dependency(%q<rspec>.freeze, [">= 0"])
      s.add_development_dependency(%q<rspec-its>.freeze, [">= 0"])
      s.add_development_dependency(%q<webmock>.freeze, [">= 0"])
    else
      s.add_dependency(%q<rack>.freeze, [">= 0"])
      s.add_dependency(%q<httpclient>.freeze, [">= 0"])
      s.add_dependency(%q<activesupport>.freeze, [">= 0"])
      s.add_dependency(%q<attr_required>.freeze, [">= 0"])
      s.add_dependency(%q<json-jwt>.freeze, [">= 1.9.0"])
      s.add_dependency(%q<rake>.freeze, [">= 0"])
      s.add_dependency(%q<simplecov>.freeze, [">= 0"])
      s.add_dependency(%q<rspec>.freeze, [">= 0"])
      s.add_dependency(%q<rspec-its>.freeze, [">= 0"])
      s.add_dependency(%q<webmock>.freeze, [">= 0"])
    end
  else
    s.add_dependency(%q<rack>.freeze, [">= 0"])
    s.add_dependency(%q<httpclient>.freeze, [">= 0"])
    s.add_dependency(%q<activesupport>.freeze, [">= 0"])
    s.add_dependency(%q<attr_required>.freeze, [">= 0"])
    s.add_dependency(%q<json-jwt>.freeze, [">= 1.9.0"])
    s.add_dependency(%q<rake>.freeze, [">= 0"])
    s.add_dependency(%q<simplecov>.freeze, [">= 0"])
    s.add_dependency(%q<rspec>.freeze, [">= 0"])
    s.add_dependency(%q<rspec-its>.freeze, [">= 0"])
    s.add_dependency(%q<webmock>.freeze, [">= 0"])
  end
end