<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="no"/>

	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="string[preceding-sibling::key[1][text() = 'CFBundleVersion' or text() = 'CFBundleShortVersionString']]">
		<string><xsl:value-of select="$VERSION" /></string>
	</xsl:template>

</xsl:stylesheet>
